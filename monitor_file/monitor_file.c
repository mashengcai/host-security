/**
 * @file  monitor_file.c
 * @brief ·À´Û¸Ä
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

void monitor_event_handle(void);
void monitor_error(int m_err, char *buf);
int main()
{
	
	pthread_t pthread_d[2] = {0};
	
	queue_t q = queue_create(128);
		
	xm_log_open();

	//monitor file process
	if(0 != pthread_create(&pthread_d[0] , NULL, inotify_watch, q))
	{
		xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__, __LINE__, " create inotify_watch err");
	}

	//monitor file process
	if(0 != pthread_create(&pthread_d[1] , NULL, inotify_execd, q))
	{
		xm_log_write(XMLOG_MONITOR_FILE, LOG_ERR, __FILE__, __FUNCTION__, __LINE__, " create inotify_execd err");
	}
	
	read_watch_conf();
		
	init_watch();
	
	while(1)
	{
		monitor_event_handle();
		//sleep(2);
	}
		
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
