
#include "net_stream.h"


#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>


int init_net_stream(const char* server_ip, int port, int&sock)
{
	struct sockaddr_in server_addr;
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		printf("socket failed");
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	bzero(&(server_addr.sin_zero),8);
	if((connect(sock,(struct sockaddr*)&server_addr,sizeof(struct sockaddr_in)))==-1)
	{
		printf("connect failed");
		return -1;
	}

	return 0;
}

void close_net_stream(int sock)
{
	if(sock>0)
	{
		close(sock);
	}
}
