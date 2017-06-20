#pragma once

#include "global.h"
#include "message.h"

void *io_send(void *arg);
void *io_recv(void* arg);
void send_msg(int sock,const TMsg& msg);
vector<TMsg> &get_recv_msgs();
