/**
* @file  hostspro.c 
* @brief 主机防护:主机防护通信接口
* @author    yu
* @date     2017-5-8
* @version  A001 
* @copyright yu
* @par History:  
	update:2017.5.9\n
	函数添加注释,┗( T﹏T )┛
*/
#include<interface.h>
#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<xm_runlog_interface.h>
#include<get_conf.h>
#include<set_conf.h>

#include <xm_queue.h>
#include <xm_monitor.h>

#define WATCH_FILE_NUM 32
#define DIR_PATH_LEN	1024

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
Pd *webdir_check();
/**********************************************************************
 * Function: bese64_encode
 * Description: base64编码
 * Parameter:
 *	bindata: 源字符串
 *	base64: 存储编码后的base64字符串
 *	binlength: 源字符串长度 
 * Return:
 * 	 0: 数据获取成功
 *	-1: 数据获取失败
 *********************************************************************/
char * base64_encode(char * bindata, char * base64, int binlength)
{
	int i, j;
	char current;

	for (i = 0, j = 0; i < binlength; i += 3)
	{
		current = (bindata[i] >> 2);
		current &= (char)0x3F;
		base64[j++] = base64char[(int)current];

		current = ((char)(bindata[i] << 4))&((char)0x30);
		if (i + 1 >= binlength)
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			base64[j++] = '=';
			break;
		}
		current |= ((char)(bindata[i+1] >> 4))&((char)0x0F);
		base64[j++] = base64char[(int)current];

		current = ((char)(bindata[i+1] << 2))&((char)0x3C );
		if (i + 2 >= binlength)
		{
			base64[j++] = base64char[(int)current];
			base64[j++] = '=';
			break;
		}
		current |= ((char)(bindata[i+2] >> 6))&((char)0x03);
		base64[j++] = base64char[(int)current];

		current = ((char)bindata[i+2])&((char)0x3F);
		base64[j++] = base64char[(int)current];
	}
	base64[j] = '\0';
	return base64;
}

//1->0->flag|close|/var/www/html/wordpress;
int monitor_check(Pd **tmp)
{
	FILE *fd = NULL;
	int i = 0;
	int ret = 0;
	
	if (-1 == access(WATCH_FILE_CONF,F_OK))
	{	
		//push_data(tmp, strlen("success"), "success");
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
		char buff[2000] = {0};
		watch_file_s conf = {{0}};
		char base_buf[1024] = {0};
		ret = fscanf(fd, "%*[^:]:%[^;];\
					%*[^:]:%[^;]; \
					%*[^:]:%[^;]; \
					%*[^:]:%[^;]; \
					%*[^:]:%[^;]; \
					", conf.file_path, conf.bak_path, conf.file_type, conf.file_switch, conf.file_flag);
		if(ret == EOF)
		{
			break;
		}
		base64_encode(conf.file_path, base_buf, strlen(conf.file_path));
		snprintf(buff, sizeof(buff), "%s|%s|%s;", conf.file_flag, conf.file_switch, base_buf);
		push_data(tmp, strlen(buff), buff);
	}

	fclose(fd);
	return 0;
}
int monitor_other(Pd **tmp, char * op, char * old_path, char * flag, char * new_path)
{
	int ret = 0;
	char buf[128] = {0};
	ret = monitor_send(op, old_path, flag, new_path, buf);
	if(ret == 0)
	{
		push_data(tmp, strlen("success"), "success");
		return 0;
	}
	else
	{
		push_data(tmp, strlen("fail"), "fail");
		if(strcmp(buf, "NULL") != 0)
		{
			push_data(tmp, strlen(buf), buf);
		}
		return -1;
	}
	return 0;
}
int monitor_send(char * op, char * old_path, char * flag, char * new_path, char *ret_buf)
{
	event_queue_s msg = {0};
	int ret = 0;
	
	if(op == NULL)
	{
		return -1;
	}
	
	msg.type = XM_TO_MONITOR;
	strcpy(msg.op, op);
	
	if(old_path != NULL)
		strcpy(msg.parm1, old_path);
	if(flag != NULL)
		strcpy(msg.parm2, flag);
	if(new_path != NULL)
		strcpy(msg.parm3, new_path);
	
	ret = xm_event_queue_send(&msg);
	if(ret != 0)
	{
		return -1;
	}
	memset(&msg, 0, sizeof(event_queue_s));
	
	ret = xm_event_queue_recv(&msg, MONITOR_TO_XM);
	if(ret != 0)
	{
		return -1;
	}
	if( strcmp(msg.parm1, "success") == 0)
	{
		return 0;
	}
	else
	{
		strcpy(ret_buf, msg.parm2);	
	}
	return -1;
}
int get_web_path(char (* path)[DIR_PATH_LEN])
{
	Pd * dataline = webdir_check();
	int i = 0;
	
	if(dataline == NULL || dataline->next == NULL || dataline->next->next == NULL || dataline->next->next->data == NULL)
	{
		strcpy(path[i++], "NULL");
		return -1;
	}
	Pd * next = dataline->next->next;
	for(; next != NULL&& next->data != NULL; next = next->next)
	{
		strcpy(path[i++], next->data);
	}
	strcpy(path[i++], "NULL");
	datalinefree(dataline);
	return 0;
}

int monitor_autoadd(Pd **tmp)
{
	FILE *fd = NULL;
	int i = 0;
	int ret = 0;
	char web_path[WATCH_FILE_NUM][DIR_PATH_LEN] = {{0}};
	char buf[128] = {0};
	if( 0 != get_web_path(web_path))
	{
		ret = -1;
	}
	
	for(i = 0; strcmp(web_path[i], "NULL") != 0; i++)
	{
		char base64_path[1024] = {0};
		base64_encode(web_path[i], base64_path, strlen(web_path[i]));
		
		//ret |= monitor_send("add", "572R56uZ55uu5b2V", base64_path, NULL, buf);
		return monitor_other(tmp,"add", "572R56uZ55uu5b2V", base64_path, NULL);
	}		
	return -1;
}
Pd *qf_monitor_file(char * op, char * old_path, char * flag, char * new_path)
{
	Pd *tmp = NULL;
	int ret = -1;
	
	if(op == NULL)
	{
		return NULL;
	}
	
	tmp = init_rt_pd();
	if(tmp == NULL)
	{
		xm_log_write(XMLOG_HOST, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "host pro tmp is null.");
		return NULL;
	}
	
	if(strcmp("check", op) == 0)		//10
	{
		ret = monitor_check(&tmp);
	}
	else if(strcmp("autoadd", op) == 0)		//10
	{
		ret = monitor_autoadd(&tmp);
	}
	else
	{
		ret = monitor_other(&tmp, op, old_path, flag, new_path);
	}
	//配置导入导出
	//set_monitor_conf(op, ret);
	
	if( ret != 0 )
	{
		xm_log_write(XMLOG_HOST, LOG_ERR, __FILE__, __FUNCTION__ ,__LINE__, "qf_monitor_file ret = %d",ret);		
		change_rt_h2(tmp,CMD_FAIL,DATA_NO);	
	}
	else
	{
		change_rt_h2(tmp,CMD_SUCCESS,DATA_NO);	
	}
	
	return tmp;
}

//1->0->flag|close|/var/www/html/wordpress;			//bas64路径 
//				->flag|open|/var/www/html/word;
//					->flag|close|/var/www/html/wor;

//void set_monitor_conf(char *op, int ret)
//{
//	int i = 0;
//	
//	if(strcmp("check", op) != 0 && ret == 0)
//	{
//		Pd *conf = NULL;
//		conf = init_rt_pd();
//		ret = monitor_check(&conf);
//		if(conf == NULL || conf->next == NULL|| conf->next->next == NULL)
//		{
//			return ;
//		}
//		conf = conf->next->next;
//		
//		for(i = 0; conf != NULL; conf = conf->next, i++ )
//		{
//			char on_off[128] = {0};
//			char path[1024] = {0};
//			char flag[128] = {0};
//			char buf[2048] = {0};
//			
//			sscanf(conf_list->data, "%[^|]|%[^|]|%[^|]", flag, on_off, path);
//			snprintf(buf, sizeof(2048), "%s %s", flag, path);
//			
//			if( 0 == strcmp(on_off, "open") )
//			{
//				set_conf(CHECK_MONITOR_FILE, i, 1, buf);	   //open
//			}
//			else if( 0 == strcmp(on_off, "close") )
//			{
//				set_conf(CHECK_MONITOR_FILE, i, 2, buf);	//close
//			}
//			
//		}
//		
//		datalinefree(conf);
//	}
//}
