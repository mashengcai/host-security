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

#include "struct.h"

#define MONITOR_SIZE (sizeof(monitor_s))

void monitor_event_handle(void);
void monitor_error(int m_err, char *buf);


monitor_s* init()
{
	monitor_s *monitor_t = MALLOC(MONITOR_SIZE);
	if(!monitor_t)
		exit(1);
	
	memset(monitor_t, 0, MONITOR_SIZE);
	
	INIT_LIST_HEAD(&monitor_t->queue);
	INIT_LIST_HEAD(&monitor_t->file);
	
	monitor_t->inotify_fd = inotify_init();
	if(monitor_t->inotify_fd <= 0)
		exit(2);
	
	monitor_t->keep_running = 1;
	
	if(!read_conf())
		exit(3);
	
	return monitor_t;
}

int main()
{
	runlog_open();
	
	monitor_s* monitor_t = init();
	
	pthread_t pthread_d[2] = {0};
	
	//watch file
	if(0 != pthread_create(&pthread_d[0] , NULL, inotify_watch, monitor_t)){
		debuginfo(ERROR, "create watch pthread error");
		exit(1);
	}

	//event handle
	if(0 != pthread_create(&pthread_d[1] , NULL, inotify_execd, monitor_t)){
		debuginfo(ERROR, "create execd pthread error");
		exit(2);
	}
	
	read_conf();
		
	init_watch();
	
	monitor_event_handle();
		
	return 0;	
}
void monitor_event_handle(void)
{
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
	switch(m_err)
	{
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
