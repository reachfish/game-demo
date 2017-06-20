#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <event.h>

#include "net_stream.h"
#include "msg_handle.h"
#include "lua_handle.h"

#define BACKLOG     5
#define MEM_SIZE    1024
#define TIME_OUT    5    
#define SAVE_FREE(p) if(p) \
		     {\
			free(p);\
			p=NULL; \
		     }

#define SAVE_DEL_EVENT(ev) if(ev) \
			   {\
			   	event_del(ev); \
				SAVE_FREE(ev); \
			   }

typedef size_t strlen_t;
typedef struct recv_msg_buf_t
{
	string msg_str;
	string len_s;
	
}recv_buf_t;

size_t min(size_t a,size_t b)
{
	return a<b?a:b;
}

string head_add_len(const string& str)
{
	const size_t buf_size = sizeof(strlen_t);
	static char len_buf[buf_size];
	strlen_t len = str.size();
	memcpy(len_buf,&len,buf_size);

	return string(len_buf,buf_size) + str;
}

vector<string> get_msgs_from_buf(recv_msg_buf_t &msg_buf, char* buf, int len)
{
	vector<string> msg_strs;
	size_t strlen_t_size = sizeof(strlen_t);
	char *end = buf + len;
	string &msg_str = msg_buf.msg_str;
	string &len_s = msg_buf.len_s;
	while(buf<end)
	{
		size_t n_ch;
		if(len_s.size()<strlen_t_size)
		{
			n_ch = min(end-buf,strlen_t_size-len_s.size());
			len_s += string(buf,n_ch);
		}
		else
		{
			strlen_t msg_len;
			memcpy(&msg_len,len_s.c_str(),sizeof(msg_len));

			n_ch = min(end-buf,msg_len-msg_str.size());
			msg_str += string(buf,n_ch);


			if(msg_str.size()==msg_len)
			{
				msg_strs.push_back(msg_str);
				msg_str.clear();
				len_s.clear();
			}
		}
		buf += n_ch;
	}
	return msg_strs;
}

struct sock_ev {
	struct event* read_ev;
	struct event* write_ev;
	struct event* timeout_ev;
	bool is_connect;
	recv_msg_buf_t msg_buf;
	vector<string> send_data;
};

static struct event_base* g_base = NULL;
static vector<TNetData> g_send_datas;
static vector<TNetData> g_recv_datas;

static map<int,struct sock_ev*> g_socks;

void release_sock_event(struct sock_ev* ev);
void on_read(int sock, short event, void* arg);
void on_write(int sock, short event, void* arg);
void on_timeout(int sock, short event, void* arg);



struct sock_ev* new_sock_event(int fd)
{
	//struct sock_ev* ev = (struct sock_ev*)malloc(sizeof(struct sock_ev));
	struct sock_ev* ev = new struct sock_ev;
	if(ev==NULL) return NULL;

	//memset(ev,0,sizeof(*ev));

	ev->read_ev = (struct event*)malloc(sizeof(struct event));
	ev->write_ev = (struct event*)malloc(sizeof(struct event));
	ev->timeout_ev = (struct event*)malloc(sizeof(struct event));

	if(!ev->read_ev||!ev->write_ev||!ev->timeout_ev)
	{
		release_sock_event(ev);
		ev = NULL;
	}

	event_set(ev->read_ev, fd, EV_READ|EV_PERSIST, on_read, ev);
	event_set(ev->write_ev, fd, EV_WRITE, on_write, ev);
	event_set(ev->timeout_ev, fd, EV_TIMEOUT, on_timeout, ev);
	event_base_set(g_base,ev->read_ev);
	event_base_set(g_base,ev->write_ev);
	event_base_set(g_base,ev->timeout_ev);

	ev->is_connect = true;
	//bzero(ev->buffer,MEM_SIZE);

	g_socks[fd] = ev;

	return ev;
}

void release_sock_event(struct sock_ev* ev)
{
	if(ev==NULL)
	{
		return;
	}

	SAVE_DEL_EVENT(ev->read_ev);
	SAVE_DEL_EVENT(ev->write_ev);
	SAVE_DEL_EVENT(ev->timeout_ev);

	SAVE_FREE(ev);
}

void on_write(int sock, short event, void* arg)
{
	struct sock_ev* ev = (struct sock_ev*)arg;

	vector<string> &send_data = ev->send_data;
	vector<string>::iterator it=send_data.begin();
	while(it!=send_data.end())
	{
		string str = head_add_len(*it);
		int ret = send(sock, str.data(), str.size(), 0);
		if(ret<0)
		{
			//release_sock_event(ev);
			ev->is_connect = false;
			break;	
		}
		it = send_data.erase(it);
	}
}

int get_fd(struct sock_ev* ev)
{
	return ev->read_ev->ev_fd;
}

void disconnect_client(sock_ev* ev)
{
	int sock = get_fd(ev);
	printf("sock[%d] disconnect\n",sock);
	release_sock_event(ev);
	close(sock);
	c_call_lua_disconnect(g_L,sock);
	g_socks.erase(sock);
}

void on_read_data(struct sock_ev* ev)
{
	
	static char buffer[MEM_SIZE];
	static recv_msg_buf_t msg_buf;
	int size;
	int sock = get_fd(ev);
	bzero(buffer, MEM_SIZE);
	size = recv(sock, buffer, sizeof(buffer), 0);
	if (size == 0) 
	{
		ev->is_connect = false;
		//disconnect_client(ev);
		return;
	}

	vector<string> msgs_str = get_msgs_from_buf(ev->msg_buf,buffer,size);
	for(size_t i=0;i<msgs_str.size();i++)
	{
		handle_one_msg(TNetData(sock,msgs_str[i]));
	}

	ev->is_connect = true;
}


void on_read(int sock, short event, void* arg)
{
	struct sock_ev* ev = (struct sock_ev*)arg;
	if(event&EV_READ)
	{
		on_read_data(ev);
	}
}

void on_timeout(int sock, short event, void* arg)
{
	struct sock_ev* ev = (struct sock_ev*)arg;
	if(!ev->is_connect)
	{
		disconnect_client(ev);
		return;
	}

	ev->is_connect = false;

	const static struct timeval timeout = {TIME_OUT,0};
	event_add(ev->timeout_ev, &timeout);
}

void on_accept(int sock, short event, void* arg)
{
	const static struct timeval timeout = {TIME_OUT,0};
	struct sockaddr_in cli_addr;
	int newfd;
	socklen_t sin_size;

	sin_size = sizeof(struct sockaddr_in);
	newfd = accept(sock, (struct sockaddr*)&cli_addr, &sin_size);
	if(newfd<=0)
	{
		return;
	}
	

	struct sock_ev* ev = new_sock_event(newfd);
	if(ev==NULL)
	{
		printf("new sock_ev fail");
		return;
	}

	
	event_add(ev->read_ev, NULL);
	event_add(ev->timeout_ev, &timeout);
}

int init_net_stream(int port)
{
	struct sockaddr_in server_addr;
	int listen_socket;

	if((listen_socket=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP))==-1)
	{
		printf("create listen socket fail");
		exit(-1);
	}
	
	//允许地址的立即重用
	int on = 1;
	setsockopt(listen_socket,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	//设置发送时限
	int time_out = 2000; //2s超时
	setsockopt(listen_socket, SOL_SOCKET, SO_SNDTIMEO,(char*)&time_out,sizeof(int));

	//设置接收时限
	setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO,(char*)&time_out,sizeof(int));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(struct sockaddr))==-1)
	{
		printf("blind failed");
		return -1;
	}

	//监听客户端连接
	if(listen(listen_socket,16)==-1)
	{
		printf("listen failed");
		return -1;
	}


	g_base = event_base_new();
	if(g_base==NULL)
	{
		printf("create event base fail");
		return -1;
	}
	const char *method = event_base_get_method(g_base);
	printf("event method:%s\n",method);

	struct event *listen_ev = (struct event*)malloc(sizeof(struct event));
	event_set(listen_ev, listen_socket, EV_READ|EV_PERSIST, on_accept, NULL);
	event_base_set(g_base,listen_ev);
	event_add(listen_ev, NULL);
	event_base_dispatch(g_base);

	return 0;
}

void send_data(int sock,const string& str)
{
	struct sock_ev *ev = g_socks[sock];
	ev->send_data.push_back(str);
	event_add(ev->write_ev,NULL);
}

