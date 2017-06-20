#include "msg_handle.h"
#include "io_service.h"

bool is_login = false;
bool is_connect = true;

void handle_one_msg(const TMsg& msg)
{
	int result;
	if(get_msg_name(msg.m_id)=="msg_sc_login" && get_msg_attr_int(msg,"result",result)==0 && result==0)
	{
		is_login = true;
	}

	if(get_msg_name(msg.m_id)!="msg_sc_heart_beat")
	{
		printf("\nrecv msg:\t");
		display_msg(msg);
		printf("\n");
	}

	is_connect = true;

	//printf("set connect true\n");

}

void handle_recv_msg()
{
	vector<TMsg>& recv_msgs = get_recv_msgs();
	for(size_t i=0;i<recv_msgs.size();i++)
	{
		handle_one_msg(recv_msgs[i]);
	}
	recv_msgs.clear();
}
