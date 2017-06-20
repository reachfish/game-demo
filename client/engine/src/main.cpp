
#include "net_stream.h"
#include "io_service.h"
#include "msg_handle.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT 12345

void *heart_beat(void* arg)
{
	int sock = *((int*)arg);
	while(true)
	{
		if(!is_connect)
		{
			printf("disconnect from server:\n");
			exit(0);
		}

		TMsg msg;
		if(create_one_msg(msg,"msg_cs_heart_beat",0)!=0)
		{
			printf("创建消息失败\n");
			continue;
		}

		is_connect = false;
		//printf("set connect false\n");
		send_msg(sock,msg);
		sleep(2);
	}
}

void *run_game(void* arg)
{
	char name[100];
	char pwd[100];

	extern bool is_login;
	int sock = *((int*)arg);
	TMsg msg;
	while(true)
	{

		if(is_login)
		{
			break;
		}

		memset(name,0,sizeof(name));
		memset(pwd,0,sizeof(pwd));
		printf("请输入用户名:");
		scanf("%s",name);
		printf("请输入密码:");
		scanf("%s",pwd);
		if(create_one_msg(msg,"msg_cs_login",name,pwd)!=0)
		{
			printf("创建消息失败\n");
			continue;
		}

		send_msg(sock,msg);
		
		sleep(1);

	}

	char word[1024];
	while(true)
	{
		printf("请输入聊天内容:\t");
		memset(word,0,sizeof(word));
		//gets(word);
		scanf("%s",word);
		if(create_one_msg(msg,"msg_cs_chart",1,word)!=0)
		{
			printf("创建消息失败\n");
			continue;
		}
		send_msg(sock,msg);


	}
	return NULL;
}

int main()
{

	int sock;
	pthread_t  io_send_service;
	pthread_t  io_recv_service;
	pthread_t  run_game_service;
	pthread_t  heart_beat_service;

		

	init_all_messages();

	if((init_net_stream(SERVER_IP,PORT,sock))!=0)
	{
		printf("init net stream failed\n");
		return -1;
	}
	
	//if(pthread_create(&io_send_service,NULL,io_send,(void*)&sock)!=0)
	//{
	//	printf("create io send service failed");
	//	return -1;
	//}
	
	if(pthread_create(&io_recv_service,NULL,io_recv,(void*)&sock)!=0)
	{
		printf("create io recv service failed");
		return -1;
	}
	
	if(pthread_create(&run_game_service,NULL,run_game,(void*)&sock)!=0)
	{
		printf("run game service failed");
		return -1;
	}

	if(pthread_create(&heart_beat_service,NULL,heart_beat,(void*)&sock)!=0)
	{
		printf("heart beat service failed");
		return -1;
	}

	//pthread_join(io_send_service,NULL);
	pthread_join(io_recv_service,NULL);
	pthread_join(run_game_service,NULL);
	pthread_join(heart_beat_service,NULL);
	

	//close_net_stream(sock);

	printf("\n");

	return 0;

}
