/**
 * @file  runlog.c 
 * @brief 日志系统
 * @author    yu
 * @date     2018-5-7
 * @version  A001 
 * @copyright yu
 * @par History: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "runlog.h"

static char run_log_file[] = "/tmp/sda_run_log.tmp";
static FILE *runlog_fp = NULL;

/** 
 * xm_log_open,打开消息队列
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-07创建
 */
int runlog_open()
{
	runlog_fp = fopen(run_log_file, "a+");

	if(runlog_fp == NULL)
		return -1;
	
	setbuf(runlog_fp, NULL);
	return 0;
}

/** 
 * debuginfo,程序运行写日志函数
 * @param[in]   logtype 	日志类型
 * @param[in]   file 		文件
 * @param[in]   fun 		函数名
 * @param[in]   line 		行号
 * @param[in]   format 		可变参数列表
 * @retval  OK      0
 * @par 修改日志
 *      yu于2018-05-07创建
 */
int debuginfo(const char * type, char *file, int line, const char * fun,  char *format, ...)
{
	if(runlog_fp == NULL)
		return 0;

	char buf[MSG_DATA_LEN] = {0};

	va_list argptr;

	va_start(argptr, format);
	vsnprintf(buf, MSG_DATA_LEN, format, argptr);
	va_end(argptr);

	//fprintf(runlog_fp, "<%s><FL:%s:%d,FUNC:%s>:%s\n", type, file, line, fun, buf);
	fprintf(runlog_fp, "<%s><%s:%d,%s>:%s\n", type, file, line, fun, buf);

	return 0;	
}

/** 
 * @retval  OK      0
 * @par 修改日志
 *      yu于2018-05-07创建
 */
void runlog_close()
{

	if(runlog_fp)
		fclose(runlog_fp);

	return ;
}

#ifdef DEBUG
int main(int argc, char **argv)
{
	int i = 1000000;
	int pid = 0;

	runlog_open();

	pid = fork();
	while(i--)
		runlog_write(1, __FILE__, __FUNCTION__, __LINE__, "-%d:i=%d", pid, i);

	runlog_close();
	return 0;
}
#endif
