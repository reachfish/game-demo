#include "message.h"


static map<string,TMsg> all_message_types;
static map<int,string>	all_message_ids;

void init_one_msg(int msg_id,char* str)
{
	const char *split = "\t";
	char *p;
	string msg_name;
	vector<TField> fields;

	if(*str=='\0')
	{
		return;
	}

	p = strtok(str,split);
	msg_name = string(p);

	while(true)
	{
		p = strtok(NULL,split);	
		if(p==NULL)
		{
			break;
		}

		string fld = string(p);
		
		size_t pos = fld.find(':');
		if(pos==string::npos)
		{
			printf("error msg field type:%s\n",p);
			return;
		}

		string field_name = fld.substr(0,pos);	

		TField field;
		char t = fld.at(pos+1);
		if(t=='i')
		{
			field.m_value = TFieldValue(0);	
		}
		else if(t=='s')
		{
			field.m_value = TFieldValue("");	
		}
		else
		{
			printf("error msg field type:%s\n",p);
			return ;
		}
		field.m_name = field_name;
		fields.push_back(field);
	}

	TMsg msg;
	msg.m_id = msg_id;
	msg.m_fields = fields;

	all_message_types[msg_name] = msg;
	all_message_ids[msg_id] = msg_name;

}

int init_all_messages()
{
	const char *msg_conf_file = "./engine/res/all_message_type.conf";
	char *line = NULL;
	size_t len = 0;

	FILE* f = fopen(msg_conf_file,"r");
	if(f==NULL)
	{
		return -1;
	}

	int msg_id = 1;
	while(getline(&line,&len,f)!=-1)
	{
		init_one_msg(msg_id,line);		
		msg_id++;
	}

	fclose(f);

	return 0;
}

int create_one_msg(TMsg& msg,const char* msg_name,...)
{
	if(all_message_types.count(msg_name)==0)
	{
		return -1;
	}
	
	msg = all_message_types[msg_name];
	vector<TField> &fields = msg.m_fields;

	va_list vl;
	va_start(vl,msg_name);
	for(size_t i=0;i<fields.size();i++)
	{
		TField& field = fields[i];
		TValueType t = field.m_value.m_t;
		switch(t)
		{
			case T_INT:
				field.m_value = TFieldValue(int(va_arg(vl,int)));
				break;
			case T_STRING:
				field.m_value =	TFieldValue(string((const char*)(va_arg(vl,const char*)))); 
				break;
			default:
				return -1;
		}
	
	}
	va_end(vl);

	return 0;
}

void pack_int(char* &p, int v)
{
	size_t len = sizeof(v);
	//v = htonl(v);
	memcpy(p,&v,len);
	p += len;
}

void pack_str(char* &p, const char* v, int len)
{
	pack_int(p, len);

	memcpy(p,v,len);
	p += len;
}


void pack_field_value(char* &p, const TFieldValue& value)
{
	switch(value.m_t)
	{
		case T_INT:
			pack_int(p,value.m_v.i);
			break;
		case T_STRING:
			pack_str(p,value.m_v.s.data(), value.m_v.s.size());
			break;
		default:
			printf("error value type:%d\n",value.m_t);
	}
}

const char* pack_msg(const TMsg& msg, size_t &len)
{
	static const int BUF_LEN = 1024;
	static char buf[BUF_LEN];

	char *p = buf;
	const vector<TField> &fields = msg.m_fields;

	pack_int(p,msg.m_id);
	for(size_t i=0;i<fields.size();i++)
	{
		pack_field_value(p,fields[i].m_value);
	}

	len = p - buf;
	*p = '\0';
	return buf;
}

int unpack_int(const char *&p, int& v)
{
	int len = sizeof(v);
	memcpy(&v,p,len);
	p += len;
 
	return 0;
}

int unpack_str(const char *&p, string& v)
{
	const int BUF_SIZE = 1024;
	static char buf[BUF_SIZE];


	int len;
	if(unpack_int(p,len)!=0)
	{
		return -1;
	}

	memcpy(buf,p,len);
	p += len;

	buf[len] = '\0';
	v = string(buf,len);

	return 0;
}

int unpack_field_value(const char *&p, TFieldValue& value)
{
	switch(value.m_t)
	{
		case T_INT:
			return unpack_int(p,value.m_v.i);
		case T_STRING:
			return unpack_str(p,value.m_v.s);
	}

	return -1;
}

int unpack(const char* data, TMsg& msg)
{
	if(data==NULL)
	{
		return -1;
	}

	const char *p = data;
	int msg_id;
	if(unpack_int(p,msg_id)!=0)
	{
		return -1;
	}
	
	if(all_message_ids.count(msg_id)==0)
	{
		return -1;
	}

	msg = all_message_types[all_message_ids[msg_id]];
	vector<TField> &fields = msg.m_fields;
	for(size_t i=0;i<fields.size();i++)
	{
		unpack_field_value(p, fields[i].m_value);
	}

	return 0;
}


int get_msg_attr(const TMsg &msg, const char* attr, TFieldValue& value)
{
	const vector<TField> &fields = msg.m_fields;
	for(size_t i=0;i<fields.size();i++)
	{
		const TField& field = fields[i];
		if(strcmp(attr,field.m_name.c_str())==0)
		{
			value = field.m_value;
			return 0;
		}
	}

	return -1;
}

int get_msg_attr_int(const TMsg &msg, const char* attr, int& value)
{
	TFieldValue fv;
	if(get_msg_attr(msg,attr,fv)!=0)
	{
		return -1;
	}

	if(fv.m_t!=T_INT)
	{
		return -1;
	}

	value = fv.m_v.i;
	return 0;
}

int get_msg_attr_str(const TMsg &msg, const char* attr, string& value)
{
	TFieldValue fv;
	if(get_msg_attr(msg,attr,fv)!=0)
	{
		return -1;
	}

	if(fv.m_t!=T_STRING)
	{
		return -1;
	}

	value = fv.m_v.s;
	return 0;
}

int set_msg_attr(TMsg &msg, const char* attr, const TFieldValue& value)
{
	vector<TField> &fields = msg.m_fields;
	for(size_t i=0;i<fields.size();i++)
	{
		TField& field = fields[i];
		if(strcmp(attr,field.m_name.c_str())==0 && value.m_t==field.m_value.m_t)
		{
			field.m_value = value;
			return 0;
		}
	}

	return -1;
}
int set_msg_attr_int(TMsg &msg, const char* attr, int value)
{
	TFieldValue fv;
	fv.m_t = T_INT;
	fv.m_v.i = value;


	int ret = set_msg_attr(msg,attr,fv);
	return ret;
}

int set_msg_attr_str(TMsg &msg, const char* attr, const string& value)
{
	TFieldValue fv;
	fv.m_t = T_STRING;
	fv.m_v.s = value;


	int ret = set_msg_attr(msg,attr,fv);
	return ret;
}
void display_msg(const TMsg &msg)
{
	const vector<TField> &fields = msg.m_fields;
	printf("[\t");
	for(size_t i=0;i<fields.size();i++)
	{
		const TField& field = fields[i];
		const TFieldValue& value = field.m_value;
		printf("%s=",field.m_name.c_str());
		if(value.m_t==T_INT)
		{
			printf("%d\t",value.m_v.i);
		}
		else if(value.m_t==T_STRING)
		{
			printf("%s\t",value.m_v.s.c_str());
		}
	}
	printf("]\n");

}

string get_msg_name(int msg_id)
{
	return all_message_ids.count(msg_id)==1 ? all_message_ids[msg_id] : "";
}

int get_msg(const string& msg_name, TMsg& msg)
{
	if(all_message_types.count(msg_name)==0)
	{
		return -1;
	}

	msg = all_message_types[msg_name];

	return 0;
}
