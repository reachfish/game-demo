#include "lua_handle.h"
#include "db_handle.h"
#include "message.h"
#include "net_stream.h"

lua_State * g_L = NULL;

int error(lua_State *L, const char *fmt, ...)
{
	char time_buf[64];

	time_t t = time(NULL);
	strftime(time_buf, sizeof(time_buf), "%d %b %H:%M:%S", localtime(&t));

	char msg[1024];
	va_list arg;
	va_start(arg,fmt);
	vsnprintf(msg,sizeof(msg), fmt, arg);
	va_end(arg);

	FILE* fp = stdout;
	fprintf(fp,"[%d] %s %s\n",getpid(),time_buf, msg);
	fflush(fp);

	if(L)
	{
		lua_close(L);
	}

	return -1;
}


void push_msg_field_value(lua_State*L,const TFieldValue& value)
{
	switch(value.m_t)
	{
		case T_INT:
			lua_pushnumber(L,value.m_v.i);
			break;
		case T_STRING:
			lua_pushstring(L,value.m_v.s.c_str());
			break;
		default:
			printf("error field type:%d\n",value.m_t);
	}
}

void push_msg_field(lua_State* L, const TField& field)
{
	lua_pushstring(L,field.m_name.c_str());
	push_msg_field_value(L,field.m_value);
	lua_settable(L,-3);
}


int c_call_lua_disconnect(lua_State *L,int sock)
{
	int narg = 1;
	int nres = 0;

	char func[] = "disconnect_client";
	lua_getglobal(L,func);
	lua_pushnumber(L,sock);
	if(lua_pcall(L,narg,nres,0)!=0)
	{
		printf("lua error:%s",lua_tostring(L,-1));
		lua_pop(L,1);
		return error(L,"error running function %s: %s", func, lua_tostring(L,-1));
	}

	return 0;
}

int c_call_lua_handle_msg(lua_State*L, int sock, const TMsg& msg)
{
	int narg = 3;
	int nres = 0;
	char func[] = "handle_msg";
	lua_getglobal(L,func);
	lua_pushnumber(L,sock);
	lua_pushstring(L,get_msg_name(msg.m_id).c_str());
	lua_newtable(L);

	const vector<TField>& fields = msg.m_fields;
	for(size_t i=0;i<fields.size();i++)
	{
		push_msg_field(L,fields[i]);
	}


	luaL_checkstack(L,1,"too many arguments");
	if(lua_pcall(L,narg,nres,0)!=0)
	{
		printf("lua error:%s",lua_tostring(L,-1));
		lua_pop(L,1);
		return error(L,"error running function %s: %s", func, lua_tostring(L,-1));
	}

	
	//nres = -nres;
	//if(!lua_istable(L,nres))
	//{
	//	return error(L, "wrong result type");
	//}
	//smsg = string(lua_tostring(L,nres));
	//
	//lua_pop(L,nres);

	return 0;
}

void load_lua_files(lua_State *L)
{
	const char* lua_files[] = {
		"main.lua",
	};
	
	for(size_t i=0;i<ARRAY_LEN(lua_files);i++)
	{
		if(luaL_loadfile(L,lua_files[i])!=0)
		{
			error(L,"loading lua file fail: %s", lua_files[i]);
		}
	}
}

lua_State *init_lua_state()
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	load_lua_files(L);
	lua_pcall(L,0,0,0);

	return L;
}


void close_lua_state(lua_State* L)
{
	lua_close(L);
}

int lua_call_c_check_user(lua_State* L)
{
	int uid = 0;
	const char *usr = luaL_checkstring(L,1);
	const char *pwd = luaL_checkstring(L,2);
	int ret = check_user(usr,pwd,uid);
	lua_pushnumber(L,ret);
	lua_pushnumber(L,uid);
	return 2;
}

int lua_call_c_send_msg(lua_State* L)
{
	size_t len;
	int sock = luaL_checknumber(L,1);
	TMsg* msg = (TMsg*)luaL_checkudata(L,2,"MyCMessage");
	const char* str = pack_msg(*msg,len);
	send_data(sock,string(str,len));

	return 0;
}

int CMessage_new(lua_State *L)
{
	int narg = 1;
	const char *msg_name = luaL_checkstring(L,narg);
	narg++;

	TMsg msg;
	if(get_msg(msg_name,msg)!=0)
	{
		return 0;
	}

	vector<TField> &fields = msg.m_fields;
	int ivalue;
	const char *strvalue;
	for(size_t i=0;i<fields.size();i++)
	{
		TFieldValue &value = fields[i].m_value;
		switch(value.m_t)
		{
			case T_INT:
				ivalue = luaL_checknumber(L,narg);
				narg++;
				set_msg_attr_int(msg,fields[i].m_name.c_str(),ivalue);			
				break;
			case T_STRING:
				strvalue = luaL_checkstring(L,narg);
				narg++;
				set_msg_attr_str(msg,fields[i].m_name.c_str(),strvalue);			
				break;
			default:
				return 0;
		}
	}

	TMsg* pMsg = (TMsg*)lua_newuserdata(L,sizeof(TMsg));
	memcpy(pMsg,&msg,sizeof(msg));

	luaL_getmetatable(L,"MyCMessage");
	lua_setmetatable(L,-2);

	return 1;
}

int CMessage_del(lua_State *L)
{
	TMsg* msg = (TMsg*)luaL_checkudata(L,1,"MyCMessage");
	delete msg;
	return 0;
}

int CMessage_pack(lua_State *L)
{
	size_t len;
	TMsg* msg = (TMsg*)luaL_checkudata(L,1,"MyCMessage");
	const char* msg_str = pack_msg(*msg,len);
	lua_pushstring(L,msg_str);
	lua_pushnumber(L,len);

	return 2;
}

int CMessage_show(lua_State *L)
{
	TMsg* msg = (TMsg*)luaL_checkudata(L,1,"MyCMessage");
	display_msg(*msg);
	return 0;
}

static const struct luaL_reg CMessage_lib[] = {
	{"new",CMessage_new},
	{"pack",CMessage_pack},
	{"del",CMessage_del},
	{"show",CMessage_show},
	{NULL,NULL}
};

void register_class_message(lua_State* L)
{
	luaL_newmetatable(L,"MyCMessage");

	lua_pushvalue(L,-1);
	lua_setfield(L,-2,"__index");

	lua_pushcfunction(L,CMessage_del);
	lua_setfield(L,-2,"__gc");

	lua_pushcfunction(L,CMessage_pack);
	lua_setfield(L,-2,"pack");

	lua_pushcfunction(L,CMessage_show);
	lua_setfield(L,-2,"show");

	luaL_register(L,"CMessage",CMessage_lib);
}

void register_lua_state(lua_State* L)
{
	lua_register(L,"check_user",lua_call_c_check_user);
	lua_register(L,"send_msg",lua_call_c_send_msg);
	register_class_message(L);
}

