#pragma once
#include <assert.h>

#define	DESCRIPTION_BUF 512
#define START_BIN 128


typedef unsigned char uchar;
typedef unsigned int  uint;

/** 
 * system rewrite
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-15创建
 */
int system_ex(const char *cmd);
/** 
 * 清空开头结尾字符
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-15创建
 */
char *clean_line(char *line);

/** 8
 * systemd info
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-18创建
 */
char *fgets_ex(char *, int , FILE *);
/** 
 * itoa
 * @retval  str
 * @par 修改日志
 *      yu于2018-06-29创建
 *      通过sprintf简单实现，
 *      后续可修改提高效率
 * 注:不可重入函数
 */
char *itoa(int n);
/** 
 * str_2_low	大写变小写字符串
 * @retval  str
 * @par 修改日志
 *      yu于2018-07-18创建
 * 		注:会改变原先字符串 所以char*p=abc这样的要小心
 */
char *str_2_low(char *buf);
/** 
 * str_2_low_ex	大写变小写字符串
 * @retval  str
 * @par 修改日志
 *      yu于2018-07-18创建
 * 		注:需要释放char
 */
char *str_2_low_ex(const char *buf);
