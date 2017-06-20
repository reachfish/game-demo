#pragma once

#include "lua_handle.h"
#include "net_stream.h"

void handle_one_msg(const TNetData &recv_data);
//void handle_recv_msgs(lua_State*L);

