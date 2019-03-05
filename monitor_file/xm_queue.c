/**
 * @file  xm_runlog_interface.c 
 * @brief 日志系统
 * @author    yu
 * @date     2017-5-23
 * @version  A001 
 * @copyright yu
 * @par History: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<unistd.h>

#include "xm_queue.h"

static int msgid = 0;
#define	XM_LOG_KEY	"/usr/share/xmirror/bin/xmirrord"

static void set_msg_value();
/** 
 * xm_log_open,打开消息队列
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2017-05-09创建
 */

void xm_event_queue_open()
{
	key_t key = ftok(XM_LOG_KEY, 1);
	if(key == -1)
	{
		fprintf(stderr, "get key failed\n");
		exit(0);
	}
	msgid = msgget(key,IPC_EXCL);  /*检查消息队列是否存在*/  
	if(msgid < 0)
	{  
		msgid = msgget(key,IPC_CREAT|0666);/*创建消息队列*/  
		if(msgid < 0)
		{  
			fprintf(stderr, "get msgid failed\n");
			exit(1);
		}
	}
	
	//设置消息队列大小
	set_msg_value();
	
	return ;
}

/** 
 * xm_log_send,发送消息队列
 * @param[in]   msg_t 	消息数据
 * @par 修改日志
 *      yu于2017-05-09创建
 */
int xm_event_queue_send(event_queue_s * msg_t)
{

	if(msgsnd(msgid, msg_t, sizeof(event_queue_s) - sizeof(long), IPC_NOWAIT) == -1) 
	{
		fprintf(stderr, "msgsnd failed\n");
		return -1;
	}
	return 0;
}

/** 
 * xm_log_recv,接收消息队列
 * @par 修改日志
 *      yu于2017-05-09创建
 */
int xm_event_queue_recv(void *arg, long type)
{
	int ret = 0;
	
	ret = msgrcv(msgid, arg, sizeof(event_queue_s) - sizeof(long), type, 0);
	if(ret == -1)
	{
		fprintf(stderr, "msgrecv err failed\n");
		return -1;
	}
	return 0;
}

/** 
 * set_msg_value,修改msg参数
 * @param[in]   level 	级别
 * @par 修改日志
 *      yu于2017-05-09创建
 */
void set_msg_value()
{	
	struct msqid_ds	msg_buf = {{0}};	

	if( msgctl(msgid, IPC_STAT, &msg_buf) == -1 )
	{
		fprintf(stderr, "get msg error \n");
		exit(0);	
	}
	if(msg_buf.msg_qbytes < XM_MSG_MAX_QUE_SIZE)
	{
		msg_buf.msg_qbytes = XM_MSG_MAX_QUE_SIZE;
		if( msgctl(msgid, IPC_SET, &msg_buf) == -1 )
		{
			fprintf(stderr, "set msg error \n");
			exit(0);	
		}
	}
}
/** 
 * xm_log_close,删除消息队列
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2017-05-09创建
 */
void xm_event_queue_close()
{
	msgctl(msgid, IPC_RMID, 0);
	return ;
}
