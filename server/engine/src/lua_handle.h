#pragma once
#include "global.h"
#include "message.h"

#ifdef __cplusplus
extern "C"
{
#endif
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

extern lua_State *g_L;

lua_State *init_lua_state();
void register_lua_state(lua_State* L);
void close_lua_state(lua_State* L);

int error(lua_State *L, const char *fmt, ...);
int c_call_lua_handle_msg(lua_State *L,int sock, const TMsg& msg);
int c_call_lua_disconnect(lua_State *L,int sock);


