
#pragma once

#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include "global.h"

typedef struct TNetData{
	int m_sock;
	string m_str;
	TNetData(int sock, const string& str):m_sock(sock),m_str(str){}
}TNetData;

int init_net_stream(int port);
int handle_net_stream();
void close_net_stream();

void send_data(int sock,const string& str);
vector<TNetData>& get_recv_datas();

