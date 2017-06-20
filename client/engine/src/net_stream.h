#pragma once

#include "global.h"

int init_net_stream(const char* server_ip, int port, int&sock);
void close_net_stream(int sock);
