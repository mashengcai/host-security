/**
 * @file  
 * @brief 
 * @author    yu
 * @date     2019-03-21
 * @version  A001 
 * @copyright yu                                                              
 */

#include <runlog.h>
#include <pthread.h>
#include <hlist.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "struct.h"
#include <stdcomm.h>

#define MONITOR_SIZE (sizeof(monitor_s))

#define CONF	"./sehost.cnf"

void monitor_event_handle(void);
void monitor_error(int m_err, char *buf);
void monitor_event_handle(void);

file_s *create_file_s(char *name, unsigned int action){

	if( 0 != access(name, F_OK))
		return NULL;

	file_s *file = MALLOC(sizeof(file_s));
	if(file == NULL)
		return NULL;
	
	file->file_name = MALLOC(strlen(name) + 1);
	printf("%s\n", name);

	file->action = action;

	strcpy(file->file_name, name);

	return file;
}

int read_conf(struct list_head *list)
{
	char buf[512] = {0};
	char path[128] = {0};
	unsigned int action = 0;

	FILE *fp = fopen(CONF, "r");
	if(!fp)
		return -1;

	while(fgets(buf, sizeof(buf), fp) != NULL){
	
		sscanf(buf, "%s %u", path, &action);

		file_s *f = create_file_s(clean_line(path), action);

		if(f == NULL){
			debuginfo(LOG_ERROR, "file_s = %s", clean_line(path));
			continue;
		}

		list_add_tail(&f->node, list);
	}

	fclose(fp);

	return 0;
}

monitor_s* init()
{
	monitor_s *monitor_t = MALLOC(MONITOR_SIZE);
	if(!monitor_t)
		exit(1);
	
	memset(monitor_t, 0, MONITOR_SIZE);
	
	monitor_t->queue = queue_create();
	INIT_LIST_HEAD(&monitor_t->file_list);
	
	monitor_t->inotify_fd = inotify_init();
	if(monitor_t->inotify_fd <= 0)
		exit(2);
	
	monitor_t->keep_running = 1;
	
	if(read_conf(&monitor_t->file_list))
		exit(3);
	
	return monitor_t;
}

void *inotify_execd(void *argv);
void* inotify_watch(void *argv);

int main()
{
	runlog_open();
	
	monitor_s* monitor_t = init();
	
	pthread_t pthread_d[2] = {0};
	
	//watch file
	if(0 != pthread_create(&pthread_d[0] , NULL, inotify_watch, monitor_t)){
		debuginfo(LOG_ERROR, "create watch pthread error");
		exit(1);
	}

	//event handle
	if(0 != pthread_create(&pthread_d[1] , NULL, inotify_execd, monitor_t)){
		debuginfo(LOG_ERROR, "create execd pthread error");
		exit(2);
	}
	
	monitor_event_handle();
		
	return 0;	
}
void monitor_event_handle(void)
{

	while(1){
		printf("running...\n");
		sleep(5);
	}

	return ;
#if 0
	int ret = 0;
	event_queue_s msg = {0};
	
	xm_event_queue_open();
	
	while(1)
	{
		ret = xm_event_queue_recv(&msg, XM_TO_MONITOR);
		
		if(ret == -1 )
		{
			continue;
		}
		
		ret = event_handle(&msg);
		
		//memset(&msg, 0, sizeof(event_queue_s));
		if(ret == 0)
		{
			strcpy(msg.parm1, "success");
		}
		else
		{
			strcpy(msg.parm1, "fail");
			monitor_error(ret, msg.parm2);
			printf("ret error = %s\n", msg.parm2);
			xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__, __LINE__, "ret error = %d\n", ret);
		}
		msg.type = MONITOR_TO_XM;
				
		xm_event_queue_send(&msg);

	}
#endif
}
void monitor_error(int m_err, char *buf)
{
	switch(m_err){
		case -1:
			strcpy(buf, "NULL");
			break;	
		case -2:
			strcpy(buf, "invalid path");//非法路径
			break;	
		case -3:
			strcpy(buf, "Path length overrun");//长度超限
			break;	
		case -4:
			strcpy(buf, "Path already exists");//已存在
			break;
		case -5:
			strcpy(buf, "Old path not exists");//需修改路径不存在
			break;
		default:
			strcpy(buf, "NULL");
			break;
	}
	return ;
}
