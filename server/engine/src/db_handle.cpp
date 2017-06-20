#include "db_handle.h"


int is_user_match(char*str, const char* usr, const char* pwd, int& uid)
{
	const char *split = "\t";
	char *p;
	p = strtok(str,split);
	if(strcmp(p,usr)!=0)
	{
		return -1;
	}

	p = strtok(NULL,split);
	if(strcmp(p,pwd)!=0)
	{
		return -1;
	}

	p = strtok(NULL,split);

	sscanf(p,"%d",&uid);

	return 0;
}

int check_user(const char* usr, const char* pwd, int& uid)
{
	const char *user_conf_file = "../engine/res/all_users.conf";
	char *line = NULL;
	size_t len = 0;

	FILE* f = fopen(user_conf_file,"r");
	if(f==NULL)
	{
		printf("no file!\n");
		return -1;
	}

	while(getline(&line,&len,f)!=-1)
	{
		if(is_user_match(line,usr,pwd,uid)==0)
		{
			return 0;
		}
	}

	fclose(f);

	return -1;
}



