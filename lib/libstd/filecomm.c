/**
 * @file  filecomm.c 
 * @brief 日志系统
 * @author    yu
 * @date     2018-5-15
 * @version  A001 
 * @copyright yu
 * @par History: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stdcomm.h"

/** 
 * 清空开头结尾字符
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-15创建
 */
char *clean_line(char *line)
{
	assert(line);

/**  注意:多个模块在使用此部份添加时需谨慎，
*	有些文件是以 . 开头 所以.不许在这里添加
* */
	while( *line == ':' || *line == ' ' || *line == '\t' || *line == '(' || 
					*line < 0 || *line > 0x7f || *line == '#' || *line == '-')		
		line++;

	int len = strlen(line);

	while( (line[len - 1] == '\n') || (line[len - 1] == '\t') || 
				(line[len - 1] == ' ') || (line[len - 1] == ')'))
		line[--len] = '\0';

	return line;
}

/** 
 * 加强版fgets_ex可以读取\的行
 * @retval  OK      s
 * @retval  ERROR   NULL
 * @par 修改日志
 *      yu于2018-06-12创建
 */
char *fgets_ex(char *s, int size, FILE *stream)
{
	char *p = NULL;
	int l = 0;
	char *c = NULL;

	p = fgets(s, size, stream);
	if(!p)
		return NULL;

	for(c = strchr(p, '\\'); c != NULL; c = strchr(p, '\\')){
		l = strlen(p);

		while( (p[l - 1] == '\n') || (p[l - 1] == '\\') || (p[l - 1] == ' '))
			p[--l] = '\0';
	
		p += l;	

		/** p++;l++; 为 \ 的行添加一个 空格 */
		*p++ = ' '; l++;

		p = fgets(p, size -= l , stream);
		if(!p)
			return s;
	}

	return s;
}

