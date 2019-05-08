#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_tools.h"

/* 检测文件类型 */
int check_file_type(const char *base)
{
	struct stat ft = {0};
	
	if( lstat(base, &ft)!=0)
		return -1;

	if( S_ISDIR(ft.st_mode) )   ///dir
		return DIR_TYPE;
	else if(S_ISREG(ft.st_mode))
		return FILE_TYPE;

	return -2;
}
