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
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "struct.h"
#include "file_tools.h"
#include <stdcomm.h>

#define MONITOR_SIZE (sizeof(monitor_s))

#define CONF	"./sehost.cnf"

void monitor_event_handle(void);
void monitor_error(int m_err, char *buf);
void monitor_event_handle(void);

int function_1(handle_s *pos);
int function_2(handle_s *pos);

void *inotify_execd(void *argv);
void* inotify_watch(void *argv);

monitor_s *g_monitor = NULL;

file_s *create_file_s(char *name, unsigned int action){

	file_s *file = NULL;
	int len = 0;

	file = MALLOC(sizeof(file_s));
	if(name == NULL || file == NULL)
		return NULL;

	len = strlen(name);

	file->file_type = check_file_type(name);
	if( file->file_type < 0){
		FREE(file);
		return NULL;
	}

	file->file_name = MALLOC(strlen(name) + 1);
	strcpy(file->file_name, name);

	if(file->file_type == DIR_TYPE && name[len - 1] == '/')
		file->file_name[len - 1] = '\0';
	
	file->action = action;

	file->f_arr[0] = function_1;

	file->f_arr[1] = function_2;
	
	file->f_num = 2;

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

#define IP 	"10.250.22.31"
#define PORT	6666 

int init_socket(const char *ip, unsigned int port)
{
	int skd = 0;
	skd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	if(skd < 0)
		return -1;

	if( 0 != connect(skd, (struct sockaddr *)&addr, sizeof(addr))){
		close(skd);
		return -2;
	}

	return skd;
}

monitor_s* init()
{
	//监控结构体
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
	
	/* conf file */
	if(read_conf(&monitor_t->file_list)){
		fprintf(stderr, "Please check conf file !!!\n");
		exit(3);
	}

	debuginfo(LOG_DEBUG, "read conf OK!");
	
	/*init socket*/
	monitor_t->sockfd = init_socket(IP, PORT);
	if(monitor_t->sockfd < 0){
		fprintf(stderr, "Please check network server !!!\n");
		exit(4);
	}

	debuginfo(LOG_DEBUG, "init network OK!");

	return monitor_t;
}

int main()
{
	runlog_open();
	
	g_monitor = init();
	
	pthread_t pthread_d[2] = {0};
	
	//watch file
	if(0 != pthread_create(&pthread_d[0] , NULL, inotify_watch, g_monitor)){
		debuginfo(LOG_ERROR, "create watch pthread error");
		exit(1);
	}

	//event handle
	if(0 != pthread_create(&pthread_d[1] , NULL, inotify_execd, g_monitor)){
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
