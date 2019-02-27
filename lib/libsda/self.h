#pragma once

/** 
 * 获取进程绝对路径
 * @retval  OK      0
 * @retval  ERROR   -1
 * @par 修改日志
 *      yu于2018-05-18创建
 */
int get_self_exe(char *path, int len);
