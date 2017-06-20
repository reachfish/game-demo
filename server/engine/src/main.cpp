#include "lua_handle.h"
#include "db_handle.h"
#include "msg_handle.h"
#include "net_stream.h"
#include "message.h"

void change_to_login_dir()
{
	char path[1024];
	chdir("./logic");

	getcwd(path,sizeof(path));
	printf("server change working dir to:%s\n", path);
}


int main()
{
	printf("server starts:\n");
	change_to_login_dir();

	//lua_State *L;
	int port = 12345;
	vector<TNetData> recv_msgs;


	init_all_messages();

	g_L = init_lua_state();
	register_lua_state(g_L);

	//初始化网络
	if(init_net_stream(port)!=0)
	{
		return -1;
	}

	while(true)
	{
		//handle_net_stream();
		//handle_recv_msgs(L);
	}


	close_lua_state(g_L);

	return 0;
}
