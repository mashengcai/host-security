/**
 * @file  monitor_file.c
 * @brief 防篡改
 * @author    yu
 * @date     2017-10-27
 * @version  A001 
 * @copyright yu                                                              
 */
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <xm_runlog_interface.h>

#include "event_queue.h"
#include "file_tools.h"
#include "inotify_watch.h"
#include "monitor_handle.h"


//watch_file_s g_watch_file_head[WATCH_FILE_NUM] = {{{0}}};
static watch_file_s *g_watch_file_head = NULL;
static watch_file_s *g_watch_file_tail = NULL;

//必须是路径，并且长度小于1024
int check_file_rule(char * path)
{
	int ret = 0;
	int len = 0;
	if(path == NULL)
	{
		return -1;
	}
	len = strlen(path);
	
	if( path[len - 1] == '/' || path[len - 1] == '\n')
	{
		path[--len] = '\0';	
	}
	
	ret = check_file_type(path);
	if(ret <=0 )
	{
		return -2;
	}
	if(len >= DIR_PATH_LEN)
	{
		return -3;	
	}
	return 0;
}

int event_handle(event_queue_s *msg)
{
	int 	ret = 0;
	char 	path[1024] = {0};
	
	if(msg->op == NULL)
	{
		return -1;
	}
	if(strcmp("open", msg->op) == 0)	
	{
		if( 0 != strcmp(msg->parm1, "all") )
		{
			Base64_Decode(path, msg->parm1, strlen(msg->parm1), 1);
			ret = check_file_rule(path);
			if(ret != 0)
			{
				if( 0 != file_exist(path) )
				{
					//监控目录已不存在
					pop_watch(path);
				
					if(write_watch_conf() != 0)
					{
						return -14;
					}
					return 0;
				}
				xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "open %s ret = %d", path, ret);
				return ret;
			}

			ret = op_open(path);//1
		}
		else
		{
			ret = op_open(msg->parm1);//1
		}
		
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "open %s ret = %d", path, ret);
	}
	else if(strcmp("close", msg->op) == 0)	
	{	
		if( 0 != strcmp(msg->parm1, "all") )
		{
			Base64_Decode(path, msg->parm1, strlen(msg->parm1), 1);
			ret = check_file_rule(path);
			if(ret != 0)
			{
				xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "close %s ret = %d", path, ret);
				return ret;
			}
			ret = op_close(path);//20
		}
		else
		{
			ret = op_close(msg->parm1);//20
		}
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "close %s ret = %d", path, ret);
	}
	else if(strcmp("add", msg->op) == 0)		
	{
		Base64_Decode(path, msg->parm2, strlen(msg->parm2), 1);
		
		ret = check_file_rule(path);
		if(ret != 0)
		{
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "add %s ret = %d", path, ret);
			return ret;
		}
		if( NULL != get_path_child_struct(path, NULL))			//重复检测  
		{
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "add %s ret = %d", path, ret);
			return -4;
		}
		ret = op_add(msg->parm1, path);//30	
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "add %s ret = %d", path, ret);
	}
	else if(strcmp("update",msg->op) == 0)	
	{
		char old_path[1024] = {0};
		char new_path[1024] = {0};
		
		Base64_Decode(old_path, msg->parm1, strlen(msg->parm1), 1);
		Base64_Decode(new_path, msg->parm3, strlen(msg->parm3), 1);
		
		ret = check_file_rule(old_path);
		if( ret != 0)
		{	
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "delete %s ret = %d", path, ret);
			return ret;
		}
		ret = check_file_rule(new_path);
		if( ret != 0)
		{	
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "delete %s ret = %d", path, ret);
			return ret;
		}
		
		if( NULL == get_path_struct(old_path))			//重复检测  
		{
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "add %s ret = %d", path, ret);
			return -5;
		}
		
		if(strcmp(old_path, new_path) == 0)
		{	//重命名
			;
		}
		else if( NULL != get_path_child_struct(new_path, old_path))		//重复检测  
		{
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "add %s ret = %d", path, ret);
			return -4;
		}
		
		ret = op_update(old_path, msg->parm2, new_path);//40
	
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "update %s ret = %d", path, ret);
	}
	else if(strcmp("delete",msg->op) == 0)	
	{
		Base64_Decode(path, msg->parm1, strlen(msg->parm1), 1);
		ret = check_file_rule(path);
		if( ret != 0)
		{	
			if( 0 != file_exist(path) )
			{
				//监控目录已不存在
				pop_watch(path);
			
				if(write_watch_conf() != 0)
				{
					return -54;
				}
				return 0;
			}
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "delete %s ret = %d", path, ret);
			return ret;
		}
		ret = op_delete(path);	//50
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "delete %s ret = %d", path, ret);
	}
	else if(strcmp("backup", msg->op) == 0)	
	{
		Base64_Decode(path, msg->parm1, strlen(msg->parm1), 1);
		ret = check_file_rule(path);
		if( ret != 0)
		{	
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "update %s ret = %d", path, ret);
			return ret;
		}
		ret = op_backup(path);	//60
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "update %s ret = %d", path, ret);
	}
	else
	{
		xm_log_write(XMLOG_MONITOR_FILE, LOG_INFO, __FILE__, __FUNCTION__ ,__LINE__, "msg->op = %s", msg->op);
		return -5;
	}
	return ret;
}
int read_watch_conf(){
	FILE *fd = NULL;
	int ret = 0;
	//xm_log_write(XMLOG_DAEMON, LOG_DEBUG,__FILE__, __FUNCTION__ ,__LINE__, "read_scan_conf");
	if (-1 == access(WATCH_FILE_CONF,F_OK))
	{
		return 0;
	}
	fd = fopen(WATCH_FILE_CONF, "r");
	if( NULL == fd)
	{
		//xm_log_write(XMLOG_DAEMON, LOG_ERR,__FILE__, __FUNCTION__ ,__LINE__, " open SCAN_CONF error");
		return -1;
	}
	while(1)
	{
		watch_file_s * conf = (watch_file_s *)malloc(sizeof(watch_file_s));
		memset(conf, 0, sizeof(watch_file_s));
		
		ret = fscanf(fd, "%*[^:]:%[^;];\
					%*[^:]:%[^;]; \
					%*[^:]:%[^;]; \
					%*[^:]:%[^;]; \
					%*[^:]:%[^;]; \
					",conf->file_path, conf->bak_path, conf->file_type, conf->file_switch, conf->file_flag);
		if(ret == EOF)
		{
			free(conf);
			break;
		}
		push_watch(conf);
	}
	fclose(fd);
	return 0;
}
int write_watch_conf()
{
	
	FILE * fd = NULL;	
	watch_file_s * conf = g_watch_file_head;

	fd = fopen(WATCH_FILE_CONF, "w");
	if( NULL == fd)
	{
		//xm_log_write(XMLOG_DAEMON, LOG_ERR, __FILE__, __FUNCTION__, __LINE__, " open WATCH_FILE_CONF error");
		return -1;
	}
	
	for(; conf != NULL; conf = conf->next)
	{

		fprintf(fd, "path:%s;bak:%s;type:%s;switch:%s;flag:%s;\n", conf->file_path, conf->bak_path, conf->file_type, conf->file_switch, conf->file_flag);
		
	}
	//xm_log_write(XMLOG_DAEMON, LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, "write_scan_conf");
	
	fclose(fd);
	
	return 0;
}
void push_watch(watch_file_s * conf)
{
	conf->next = NULL;
	if(g_watch_file_tail)
    {
       g_watch_file_tail->next = conf;
       g_watch_file_tail = conf;
    }
	else
    {
      g_watch_file_head = g_watch_file_tail = conf;
    }
}
void pop_watch(const char * path)
{
	watch_file_s ** conf = NULL;
	watch_file_s * list = NULL;
	for(conf = &g_watch_file_head; *conf != NULL; conf = &((*conf)->next))
	{
		if( strcmp( (*conf)->file_path, path) == 0)
		{
			list = *conf;
			
			if(g_watch_file_head == list)
			{
				g_watch_file_head = list->next;
			}
			if(g_watch_file_head == NULL)
			{
				g_watch_file_tail = g_watch_file_head;
			}
			else if(g_watch_file_tail == list)
			{
				//g_watch_file_tail = (watch_file_s *)((char *)conf - ((char *)((watch_file_s *)0)->next));
				g_watch_file_tail = (watch_file_s *)((char *)conf - ((unsigned long)&((watch_file_s *)0)->next));
				//g_watch_file_tail = list_entry(conf, watch_file_s, next);
			}
			*conf = list->next;
			free(list);
			return;
		}
	}
}
int init_watch()
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if( 0 == strcmp(conf->file_switch, "close" ) )
		{
			continue;
		}
		if( 0 == file_exist(conf->bak_path) )
		{
			delete_dir_tree(conf->bak_path);
		}
	
		
		path_change_md5path(conf->file_path, conf->bak_path);
		//备份
		copy_dir(conf->file_path, conf->bak_path);
		
		if( 0!= watch_file_interface(conf->file_path))
		{
			return -2;
		}
	}
	return 0;
}
int op_add(const char * flag, const char * path)
{
	int file_type = 0;
	
	if(path == NULL || flag == NULL)
	{
		return -31;
	}
	
	watch_file_s * conf = (watch_file_s *)malloc(sizeof(watch_file_s));
	memset(conf, 0, sizeof(watch_file_s));
	//g_watch_file_head + gs_watch_file_num;
	
	file_type = check_file_type(path);
	
	memcpy( conf->file_path, path, DIR_PATH_LEN - 1);
	strncpy( conf->file_switch, "open", 15);
	strncpy( conf->file_flag, flag, 127);
	path_change_md5path(conf->file_path, conf->bak_path);
	
	copy_dir(conf->file_path, conf->bak_path);
		
	if(file_type == WATCH_DIR)
	{
		strcpy( conf->file_type, "dir");
	}
	else if(file_type == WATCH_FILE)
	{		
		strcpy( conf->file_type, "file");
	}
	
	if( 0!= watch_file_interface(conf->file_path))
	{
		return -32;
	}
	
	push_watch(conf);

	if( 0!= write_watch_conf())
	{
		return -33;
	}
	
	return 0;
}

//50
int op_delete(const char * path)
{
	char *bak_path = NULL;
	char *web_path = NULL;
	
	if(path == NULL)
	{
		return -51;
	}
	
	if( 0 == file_exist(bak_path) )
	{
		if(rm_watch_interface(bak_path) != 0)
		{
			printf("rm watch %s\n", bak_path);
			return -53;
		}
		delete_dir_tree(bak_path);
	}
	
	get_path_head_with_file(path, &bak_path, &web_path);
	
	if( 0 == file_exist(bak_path) )
	{
		if(rm_watch_interface(bak_path) != 0)
		{
			printf("rm watch %s\n", bak_path);
			return -53;
		}
		delete_dir_tree(bak_path);
	}
	
	pop_watch(path);
	
	if(write_watch_conf() != 0)
	{
		return -54;
	}
	
	return 0;
}
//10
int op_open(const char * path)
{
	if(path == NULL)
	{
		return -11;
	}
	if(strcmp(path, "all") == 0)
	{
		return open_watch_all();
	}
	return open_watch_path(path);
}
int open_watch_path(const char * path)
{
	watch_file_s * conf = NULL;

	conf = get_path_struct(path);
	if(conf == NULL)
	{
		return -12;
	}
		
	strcpy( conf->file_switch, "open");

	path_change_md5path(conf->file_path, conf->bak_path);
	
	copy_dir(conf->file_path, conf->bak_path);
	
	if( 0 != watch_file_interface(conf->file_path))
	{
		return -13;
	}
	
	if( 0!= write_watch_conf())
	{
		return -14;
	}
	return 0;	
}
int open_watch_all()
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if(strcmp(conf->file_switch, "close") == 0)
		{
			if( 0 != open_watch_path(conf->file_path) )
			{
				return -18;
			}
		}
	}
	return 0;
}
int op_close(const char * path)
{
	if(path == NULL)
	{
		return -21;
	}
	if(strcmp(path, "all") == 0)
	{
		return close_watch_all();
	}
	return close_watch_path(path);
}
int close_watch_path(const char * path)
{
	watch_file_s * conf = NULL;
	char *bak_path = NULL;
	char *web_path = NULL;
	
	conf = get_path_struct(path);
	if(conf == NULL)
	{
		return -22;
	}
	
	if(path == NULL)
	{
		return -23;
	}
	//pop_watch(path);
	
	get_path_head_with_file(path, &bak_path, &web_path);
	
	strcpy( conf->file_switch, "close");

	if( 0 != rm_watch_interface(bak_path))
	{
		return -25;
	}
	
	if( 0 == file_exist(bak_path) )
	{
		delete_dir_tree(bak_path);
	}
	
	if( 0!= write_watch_conf())
	{
		return -26;
	}
	return 0;	
}
int close_watch_all()
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if(strcmp(conf->file_switch, "open") == 0)
		{
			if( 0 != close_watch_path(conf->file_path) )
			{
				return -28;
			}
		}
	}
	return 0;
}
int op_backup(const char * path)
{
	char *bak_path = NULL;
	char *web_path = NULL;
	
	get_path_head_with_file(path, &bak_path, &web_path);
	if( 0 == file_exist(bak_path) )
	{
		delete_dir_tree(bak_path);
	}
	//备份
	copy_dir(path, bak_path);
	return 0;
}
int op_update(const char * old_path, const char * flag, const char * new_path)
{
	int ret = 0;
	ret = op_delete(old_path);
	if(ret != 0)
	{
		return ret + 10;
	}
	ret = op_add(flag, new_path);
	if(ret != 0)
	{
		return ret + 30;
	}
	return 0;
}
watch_file_s * get_path_struct(const char * path)
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if( 0 == strcmp( conf->file_path, path) )
		{
			return conf;
		}
	}
	return NULL;
}
watch_file_s * get_path_child_struct(const char * path, const char * old_path)
{
	watch_file_s * conf = g_watch_file_head;

	for(; conf != NULL; conf = conf->next)
	{
		if(old_path != NULL)
		{
			if(strcmp(old_path, conf->file_path) == 0)
			{
				continue;	
			}	
		}
		if ( 1 == check_child_path(conf->file_path, path) )//是子目录
		{
			return conf;
		}
	}
	
	return NULL;
}
int check_file_watch(const char * path)
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if( 	(0 == memcmp(conf->file_path, path, strlen(path))) &&	//文件名
				(0 == strcmp(conf->file_type, "file")				) &&				//类型	
				(0 == strcmp(conf->file_switch, "open")				) )	//开关
			
		{
			return 0;
		}
	}
	return -1;
}
int get_path_head_with_bak(const char * path, char **bak_path, char **file_path)
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if( 0 == memcmp( conf->bak_path, path, strlen(conf->bak_path) ) )
		{
			*bak_path 	= 	conf->bak_path;
			*file_path	= 	conf->file_path;
			return 0;
		}
	}
	return -1;
}
int get_path_head_with_file(const char * path, char **bak_path, char **file_path)
{
	watch_file_s * conf = g_watch_file_head;
	for(; conf != NULL; conf = conf->next)
	{
		if( 0 == memcmp( conf->file_path, path, strlen(conf->file_path)) )
		{
			*bak_path 	= 	conf->bak_path;
			*file_path	= 	conf->file_path;
			return 0;
		}
	}
	return -1;
}
