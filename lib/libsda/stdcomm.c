/**
 * @file  stdcomm.c 
 * @brief 通用系统库
 * @author    yu
 * @date     2018-6-29
 * @version  A001 
 * @copyright yu
 * @par History: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#include "stdcomm.h"

/** 
 * itoa
 * @retval  str
 * @par 修改日志
 *      yu于2018-06-29创建
 *      通过sprintf简单实现，
 *      后续可修改提高效率
 * 注:不可重入函数
 */
char *itoa(int n)
{
	static char buf[64];
	memset(buf, 0, 64);
	sprintf(buf, "%d", n);
	return buf;
}

/** 
 * str_2_low	大写变小写字符串
 * @retval  str
 * @par 修改日志
 *      yu于2018-07-18创建
 * 		注:会改变原先字符串 所以char*p=abc这样的要小心
 */
char *str_2_low(char *buf)
{
	char *ptr = buf;

	for(; ptr && *ptr; ptr++)
		*ptr = tolower(*ptr);

	return buf;
}

/** 
 * str_2_low_ex	大写变小写字符串
 * @retval  str
 * @par 修改日志
 *      yu于2018-07-18创建
 * 		注:需要释放char
 */
char *str_2_low_ex(const char *buf)
{
	return str_2_low(strdup(buf));
}

/** 
 * system rewrite
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-15创建
 */
int system_ex(const char *cmd)
{
	assert(cmd);

	int status = system(cmd);
	
	if(status == -1)
		return -1;

	/* WIFEXITED(status) != 0 && WEXITSTATUS(status) == 0*/
	if(WIFEXITED(status) && !WEXITSTATUS(status))	
			return 0;

	return -2;	
}
