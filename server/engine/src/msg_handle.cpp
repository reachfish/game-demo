#include "msg_handle.h"
#include "net_stream.h"


void handle_one_msg(const TNetData &recv_data)
{
	
	TMsg msg;
	if(unpack(recv_data.m_str.data(),msg)==0)
	{
		c_call_lua_handle_msg(g_L,recv_data.m_sock, msg);
	}
	else
	{
		printf("unpack msg fail:[%s][len:%ld]\n",recv_data.m_str.data(),recv_data.m_str.size());
	}
}

void handle_recv_msgs(lua_State*L)
{
	//vector<TNetData>& recv_datas = get_recv_datas();
	//for(size_t i=0;i<recv_datas.size();i++)
	//{
	//	handle_one_msg(L,recv_datas[i]);
	//}

	//recv_datas.clear();
}
