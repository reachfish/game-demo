#pragma once

#include "global.h"

typedef enum{
	T_INT,
	T_STRING,
}TValueType;

typedef struct TFieldValue{
	TValueType m_t;
	struct{
		int i;
		string s;
	}m_v;
	
	TFieldValue() { m_t = T_INT; m_v.i = 0; }
	TFieldValue(int i) { m_t = T_INT; m_v.i = i; }
	TFieldValue(const string& s) { m_t = T_STRING; m_v.s = s; }
}TFieldValue;

typedef struct TField{
	string m_name;
	TFieldValue m_value;
}TField;

typedef struct {
	int m_id;
	vector<TField> m_fields;
}TMsg;



int init_all_messages();
int create_one_msg(TMsg& msg,const char* msg_name,...);
const char* pack_msg(const TMsg& msg, size_t& len);
int unpack(const char* data, TMsg& msg);
void display_msg(const TMsg& msg);
int get_msg_attr_int(const TMsg &msg, const char* attr, int& value);
int get_msg_attr_str(const TMsg &msg, const char* attr, string& value);
int set_msg_attr_int(TMsg &msg, const char* attr, int value);
int set_msg_attr_str(TMsg &msg, const char* attr, const string& value);
string get_msg_name(int msg_id);
int get_msg(const string& msg_name, TMsg& msg);
