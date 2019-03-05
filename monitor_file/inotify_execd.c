#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <assert.h>
#include <errno.h>

#include <sys/inotify.h>

#include "event_queue.h"
#include "file_list.h"
#include "file_tools.h"
#include "inotify_watch.h"

extern int keep_running;
//static int watched_items;

/* Create an inotify instance and open a file descriptor
   to access it */

extern int inotify_fd;
#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF)

static void handle_events (queue_t q);
static void handle_event(queue_entry_t event);
static int filepath_get_bakpath(char *filepath, char * bakpath);

static int file_modify_event(const char* file_name, const char* bak_name);
static int file_delete_event(const char* file_name, const char* bak_name);
static int file_create_event(const char* file_name, const char* bak_name);
static int file_recover(const char* file_name, unsigned int wd, unsigned long flags);
static int file_delete_self_event(const char* file_name, const char* bak_name);
static int file_ignored_event(unsigned int wd);
static int file_move_self_event(const char* file_name, const char* bak_name);

static int dir_create_event(const char* file_name, unsigned int wd);
static int dir_move_to_event(const char* file_name, unsigned int wd);
static int dir_recover(const char* file_name, unsigned int wd, unsigned long flags);

void handle_debug (queue_entry_t event)
{
	/* If the event was associated with a filename, we will store it here */
	char *cur_event_filename = NULL;
	char *cur_event_file_or_dir = NULL;
	/* This is the watch descriptor the event occurred on */
	int cur_event_wd = event->inot_ev.wd;
	int cur_event_cookie = event->inot_ev.cookie;
	unsigned long flags;

	if (event->inot_ev.len)
	{
		cur_event_filename = event->inot_ev.name;
	}

	flags = event->inot_ev.mask & 
		~(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED );

	if ( event->inot_ev.mask & IN_ISDIR )
	{
		cur_event_file_or_dir = "Dir";
	}
	else 
	{
		cur_event_file_or_dir = "File";
	}
	
	/* Perform event dependent handler routines */
	/* The mask is the magic that tells us what file operation occurred */
	switch (event->inot_ev.mask & 
			(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED))
	{
		/* File was accessed */
		case IN_ACCESS:
			printf ("ACCESS: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File was modified */
		case IN_MODIFY:
			printf ("MODIFY: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File changed attributes */
		case IN_ATTRIB:
			printf ("ATTRIB: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File open for writing was closed */
		case IN_CLOSE_WRITE:
			printf ("CLOSE_WRITE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File open read-only was closed */
		case IN_CLOSE_NOWRITE:
			printf ("CLOSE_NOWRITE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File was opened */
		case IN_OPEN:
			printf ("OPEN: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File was moved from X */
		case IN_MOVED_FROM:
			printf ("MOVED_FROM: %s \"%s\" on WD #%i. Cookie=%d\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd, 
					cur_event_cookie);
			break;

			/* File was moved to X */
		case IN_MOVED_TO:
			printf ("MOVED_TO: %s \"%s\" on WD #%i. Cookie=%d\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd, 
					cur_event_cookie);
			break;

			/* Subdir or file was deleted */
		case IN_DELETE:
			printf ("DELETE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Subdir or file was created */
		case IN_CREATE:
			printf ("CREATE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Watched entry was deleted */
		case IN_DELETE_SELF:
			printf ("DELETE_SELF: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Watched entry was moved */
		case IN_MOVE_SELF:
			printf ("MOVE_SELF: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Backing FS was unmounted */
		case IN_UNMOUNT:
			printf ("UNMOUNT: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Too many FS events were received without reading them
			   some event notifications were potentially lost.  */
		case IN_Q_OVERFLOW:
			printf ("Warning: AN OVERFLOW EVENT OCCURRED: \n");
			break;

			/* Watch was removed explicitly by inotify_rm_watch or automatically
			   because file was deleted, or file system was unmounted.  */
		case IN_IGNORED:
			/*watched_items--;*/
			printf ("IGNORED: WD #%d\n", cur_event_wd);
			/* printf("Watching = %d items\n",watched_items); */
			break;

			/* Some unknown message received */
		default:
			printf ("UNKNOWN EVENT \"%X\" OCCURRED for file \"%s\" on WD #%i\n",
					event->inot_ev.mask, cur_event_filename, cur_event_wd);
			break;
	}
	/* If any flags were set other than IN_ISDIR, report the flags */
	if (flags & (~IN_ISDIR))
	{
		flags = event->inot_ev.mask;
		printf ("Flags=%lX\n", flags);
	}
}

void handle_event (queue_entry_t event)
{

	handle_debug(event);
	/* If the event was associated with a filename, we will store it here */
	char *cur_event_filename = NULL;
	//char *cur_event_file_or_dir = NULL;
	/* This is the watch descriptor the event occurred on */
	int cur_event_wd = event->inot_ev.wd;
	//int cur_event_cookie = event->inot_ev.cookie;
	int ret = 0;
	unsigned long flags;

	if (event->inot_ev.len)
	{
		cur_event_filename = event->inot_ev.name;
	}

	//flags = event->inot_ev.mask & 
	//~(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED );

	flags = event->inot_ev.mask & 
		(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED );

	if(flags == IN_IGNORED)
	{
		ret = file_ignored_event(cur_event_wd);
	}
	else if ( event->inot_ev.mask & IN_ISDIR )
	{
		//	cur_event_file_or_dir = "Dir";
		ret = dir_recover(cur_event_filename, cur_event_wd, flags);
	}
	else 
	{
		//	cur_event_file_or_dir = "File";
		ret = file_recover(cur_event_filename, cur_event_wd, flags);
	}


	if(ret < 0)
	{
		printf("path=%s,wd=%d,flags=%ld errno=%d\n", cur_event_filename, cur_event_wd, flags, ret);
	}
	return ;
}
void handle_events (queue_t q)
{
	queue_entry_t event;
	while (!queue_empty (q))
	{
		event = queue_dequeue (q);
		handle_event (event);
		free (event);
	}
}

void *inotify_execd(void *argv)
{
	queue_t q = (queue_t)argv;
	while(1)
	{
		handle_events(q);
		//sleep(1);
		usleep(100000);
	}

	return NULL;
}

int filepath_get_bakpath(char *filepath, char * bakpath)
{
	int dir_path_len = 0;
	char * bak_head_path 	= NULL;
	char * web_head_path	= NULL;
	
	int ret = get_path_head_with_file(filepath, &bak_head_path, &web_head_path);
	if(ret != 0)
	{
		assert(0);
		return -1;
	}
	
	dir_path_len = strlen(web_head_path);
	
	if( filepath[dir_path_len] == '\0') //filepath就是根目录
	{
		snprintf(bakpath, DIR_PATH_LEN, "%s", bak_head_path);
		return 0;
	}

	snprintf(bakpath, DIR_PATH_LEN, "%s%s",bak_head_path, filepath + dir_path_len );

	return 0;
}
//1101
int file_modify_event(const char* web_path, const char * bak_path)
{
	int ret = 0;

	//检测是否有此文件
	//access
	if( access(bak_path, F_OK) < 0)
	{
		//不纯在，删除文件
		ret = delete_file(web_path);
		if(ret != 0)
		{
			return -1102;
		}
	}

	//校验MD5是否一致 1120
	ret = check_two_file_md5(web_path, bak_path);
	if(ret == 1)
	{
		ret = delete_file(web_path);
		if(ret != 0)
		{
			return -1103;
		}
	}
	//printf("modify file_path=%s, back_path=%s\n",web_path ,bak_path);
	return 0;
}
int file_delete_event(const char* web_path, const char * bak_path)
{

	int ret = 0;
	char fa_path[DIR_PATH_LEN]	= 	{0};

	memcpy(fa_path, bak_path, sizeof(fa_path));
	//检测是否有此文件
	//access
	if( access(bak_path, F_OK) < 0)
	{
		//不纯在
		return 0;
	}

	//检测目录是否存在，如果不纯在返回
	if( access(dirname(fa_path), F_OK) < 0)
	{
		//不纯在
		return 0;
	}
	//目录存在
	ret = copy_file(bak_path, web_path);
	if(ret != 0)
	{
		return -1203;
	}
	//printf("delete file_path=%s, back_path=%s\n",web_path ,bak_path);
	return 0;
}
int file_create_event(const char* web_path, const char * bak_path)
{
	int ret = 0;

	//检测是否有此文件
	//access
	if( access(bak_path, F_OK) < 0)
	{
		//不纯在，删除文件
		ret = delete_file(web_path);
		if(ret != 0)
		{
			return -1302;
		}
		return 0;

	}

	//校验MD5是否一致 1120
	ret = check_two_file_md5(web_path, bak_path);//0 一致，1不一致，其他，错误
	if(ret == 1)
	{
		ret = delete_file(web_path);
		if(ret != 0)
		{
			return -1303;
		}
	}
	else if(ret != 0)
	{
		return -1304;
	}
	
	//检测文件是否在监控列表
	ret = check_file_watch(web_path);
	if(ret == 0)
	{
		//需要监控
		watch_file(web_path);
	}
	//printf("create file_path=%s, back_path=%s\n",web_path ,bak_path);
	return 0;
}
//1401
int file_delete_self_event(const char* web_path, const char * bak_path)
{
	char fa_path[DIR_PATH_LEN]	= 	{0};
	int file_type = 0;
	int ret = 0;
	
	memcpy(fa_path, bak_path, sizeof(fa_path));
	//检测是否有此目录
	//access
	if( access(bak_path, F_OK) < 0)
	{
		return 0;
	}

	file_type = check_file_type(bak_path);
	//检测是否有此目录
	if( file_type == 1 && access(web_path, F_OK) == 0)
	{
		return 0;
	}
	else if(file_type == 2 && access(web_path, F_OK) == 0)
	{
		ret = check_two_file_md5(web_path, bak_path);//0 一致，1不一致，其他，错误
		if(ret == 1)
		{
			ret = delete_file(web_path);
			if(ret != 0)
			{
				return -1403;
			}
		}
	}

	//检测是否有父目录
	if(access( dirname(fa_path), F_OK) == 0)
	{
		//创建目录 ,并拷贝子文件
		if( 0!= copy_dir(bak_path, web_path) )
		{
			return -1402;
		}
	}
	//首先判断是否是跟目录		
	char * bak_head_path 	= NULL;
	char * web_head_path	= NULL;
	
	get_path_head_with_file(web_path, &bak_head_path, &web_head_path);
	if(ret != 0)
	{
		assert(0);
		return -1404;
	}
	if(strcmp(web_path, web_head_path) == 0)
	{
		if( 1 == file_type )//dir
		{
			watch_tree_dir(web_head_path);
		}
		else if(2 == file_type)//file
		{
			watch_file(web_head_path);
		}
		
	}
	return 0;
}

//1501
int file_move_self_event(const char* web_path, const char * bak_path)
{
	char fa_path[DIR_PATH_LEN]	= 	{0};

	memcpy(fa_path, bak_path, sizeof(fa_path));

	//检测是否有此目录
	if( access(web_path, F_OK) == 0)
	{
		return 0;
	}
	else
	{
		//检测是否有父目录
		if( access( dirname(fa_path), F_OK) == 0)
		{
			//创建目录 ,并拷贝子文件
			if( 0!= copy_dir(bak_path, web_path) )
			{
				return -1502;
			}
			//return 0;
		}
	}
	//释放节点WD
	rm_watch_interface(bak_path);	
	
	char * bak_head_path 	= NULL;
	char * web_head_path	= NULL;
	
	int ret = get_path_head_with_file(web_path, &bak_head_path, &web_head_path);
	if(ret != 0)
	{
		assert(0);
		return -1503;
	}
	//首先判断是否是跟目录
	if(strcmp(web_path, web_head_path) == 0)
	{
		int ret = check_file_type(web_head_path);
		if( 1 == ret )//dir
		{
			watch_tree_dir(web_head_path);
		}
		else if(2 == ret)//file
		{
			watch_file(web_head_path);
		}
	}

	return 0;
}
int file_ignored_event(unsigned int wd)
{
	//释放map信息
	int ret = 0;
	
	hash_pop(wd);

	if(ret != 0)
	{
		return -1601;
	}
	return 0;
}

int file_recover(const char* file_name, unsigned int wd, unsigned long flags)
{
	//正则过滤，如果不是指定类型，或者路径是目录
	char * dir_path	 		= 	NULL;
	char bak_path[DIR_PATH_LEN]	= 	{0};
	char web_path[DIR_PATH_LEN] 	= 	{0};
	int file_type = 0;
	dir_path = hash_get_value(wd);
	if(dir_path == NULL)
	{
		return -1004;
	}

	if(file_name == NULL)
	{
		snprintf(web_path, DIR_PATH_LEN, "%s", dir_path);
	}
	else
	{
		snprintf(web_path, DIR_PATH_LEN, "%s/%s", dir_path, file_name);
	}

	filepath_get_bakpath(web_path, bak_path);
	//检测 文件名后缀
	file_type = check_file_type(web_path);
	
	//是文件，单不是后缀中的文件
	if( file_type == -1)
	{
		//此文件被删除
		file_type = check_file_type(bak_path);
		if( file_type == -1)
		{
			//备份文件也无，那么返回吧
			return 0;
		}
	}
	
	if( (file_type == 2) && (0 != check_file_postfix(web_path)) )
	{
		printf("web_path:%s = %d\n", web_path, 1);
		return 0;
	}
	printf("web_path:%s = %d\n", web_path, 0);
	
	switch (flags)
	{		
		case IN_MODIFY:			//修改 	error -1101 start
			return file_modify_event(web_path, bak_path);

		case IN_MOVED_FROM:		//移出		
		case IN_DELETE:			//删除	error -1201 start
			return file_delete_event(web_path, bak_path);

		case IN_CREATE:			//创建
		case IN_MOVED_TO:		//移入	error -1301 start
			return file_create_event(web_path, bak_path);

		case IN_DELETE_SELF:	//删除	error -1401 start	
			return file_delete_self_event(web_path, bak_path);	

		case IN_MOVE_SELF:		//移出	error -1501 start
			return file_move_self_event(web_path, bak_path);
		default:
			return -1002;
	}
	return -1003;
}

//2101
int dir_create_event(const char* file_name, unsigned int wd)
{
	char * dir_path	 		= 	NULL;
	char bak_path[DIR_PATH_LEN]	= 	{0};
	char web_path[DIR_PATH_LEN] 	= 	{0};
	int ret = 0;

	dir_path = hash_get_value(wd);
	if(dir_path == NULL)
	{
		return -2101;
	}
	snprintf(web_path, DIR_PATH_LEN, "%s/%s", dir_path, file_name);

	filepath_get_bakpath(web_path, bak_path);

	//检测是否有此文件夹
	//access
	if( access(bak_path, F_OK) < 0)
	{
		/*不纯在，删除目录*/
		ret = delete_dir_tree(web_path);
		if(ret != 0)
		{
			return -2102;
		}
		return 0;			
	}

	//添加目录树监控
	ret = watch_tree_dir(web_path);

	if(ret != 0)
	{
		return -2104;
	}
	return 0;
}

int dir_move_to_event(const char* file_name, unsigned int wd)
{
	char * dir_path	 		= 	NULL;
	char web_path[DIR_PATH_LEN] 	= 	{0};
	//int ret = 0;

	dir_path = hash_get_value(wd);
	if(dir_path == NULL)
	{
		return -2301;
	}
	snprintf(web_path, DIR_PATH_LEN, "%s/%s", dir_path, file_name);

	//删除目录树
	delete_dir_tree(web_path);

	return 0;
}

int dir_recover(const char* file_name, unsigned int wd, unsigned long flags)
{

	assert(file_name != NULL);

	if(file_name == NULL)
	{
		return -2001;
	}

	switch (flags)
	{		
		case IN_CREATE:				//创建	error -2101 start
			return dir_create_event(file_name, wd);

		case IN_MOVED_TO:				//移入	error -2301 start
			return dir_move_to_event(file_name, wd);
		case IN_MOVED_FROM:
		case IN_DELETE:				//保留，这哪是无用
			return 0;

		default:
			return -2002;
	}
	return -2003;
}
