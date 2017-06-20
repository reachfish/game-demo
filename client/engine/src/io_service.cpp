#include "io_service.h"
#include <sys/socket.h>
#include <vector>
#include "msg_handle.h"

#define BUF_SIZE 1024

using std::vector;

typedef size_t strlen_t;
typedef struct recv_msg_buf_t
{
	string msg_str;
	string len_s;

}recv_buf_t;

static vector<TMsg> g_send_msgs;
static vector<TMsg> g_recv_msgs;

size_t min(size_t a,size_t b)
{
	return a<b?a:b;
}

string head_add_len(const string& str)
{
	static char len_buf[sizeof(strlen_t)];
	strlen_t len = str.size();
	memcpy(len_buf,&len,sizeof(len));

	string ret = string(len_buf,sizeof(len_buf)) + str;
	return ret;
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

void *io_send(void *arg)
{
	int sock = *((int*)arg);

	while(true)
	{
		sleep(0.01);
		size_t len;
		for(size_t i=0;i<g_send_msgs.size();i++)
		{
			const char* buf = pack_msg(g_send_msgs[i],len);
			string str = head_add_len(string(buf,len));
			if(send(sock,str.data(),str.size(),0)<0)
			{
			
				printf("send failed\n");
			}
			else
			{
				//printf("send[%ld/%ld]:\t",i,g_send_msgs.size());
				//display_msg(g_send_msgs[i]);
				//printf("[%s][len:%ld]\n",str.data(),str.size());
			}
		}
		g_send_msgs.clear();
	}

	return NULL;
}


void *io_recv(void* arg)
{
	static char recv_buf[BUF_SIZE];
	static recv_msg_buf_t msg_buf;
	
	int sock = *((int*)arg);
	int ret;
	while(true)
	{
		sleep(0.01);
		ret = recv(sock,recv_buf,sizeof(recv_buf),0);
		if(ret<=0)
		{
			continue;
		}

		if(ret<BUF_SIZE)
		{
			memset(&recv_buf[ret],0,1);
		}

		vector<string> msgs_str = get_msgs_from_buf(msg_buf,recv_buf,ret);
		for(size_t i=0;i<msgs_str.size();i++)
		{
			TMsg msg;
			const char* str = msgs_str[i].data();
			if(unpack(str,msg)==0)
			{
				g_recv_msgs.push_back(msg);
				//display_msg(g_send_msgs[i]);
			}
			else
			{
				printf("unpack msg fail:%s\n",str);
			}
		}

		handle_recv_msg();
		//printf("recv msg:%s\n",recv_buf);
	}
	
	return NULL;
}


void send_msg(int sock,const TMsg& msg)
{
	size_t len;
	const char* buf = pack_msg(msg,len);
	string str = head_add_len(string(buf,len));
	if(send(sock,str.data(),str.size(),0)<0)
	{
	
		printf("send failed\n");
	}
	else
	{
		//printf("send[%ld/%ld]:\t",i,g_send_msgs.size());
		//display_msg(g_send_msgs[i]);
		//printf("[%s][len:%ld]\n",str.data(),str.size());
	}
	//g_send_msgs.push_back(msg);
}

vector<TMsg> &get_recv_msgs()
{
	return g_recv_msgs;
}

