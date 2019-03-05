#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <sys/stat.h>
#include <libgen.h>
#include <regex.h> 
#include <assert.h>
#include "file_tools.h"

int get_file_md5(const char *file, char *buf);

int delete_file(const char* const path)
{
	
	if( access(path, F_OK) < 0)
	{
		return 0;
	}
	printf("delete  %s\n", path);
	return remove(path);
}
int check_two_file_md5(const char * webpath, const char *bakpath)
{
	char web_md5[33] = {0};
	char bak_md5[33] = {0};
	
	if( 0 != get_file_md5(webpath, web_md5) )
	{
		return -1;
	}
	
	if( 0 != get_file_md5(bakpath, bak_md5) )
	{
		return -2;
	}
	if( 0 != memcmp(web_md5, bak_md5, 33) )
	{
		return 1;
	}
	return 0;
}
int get_file_md5(const char *file, char *buf)  
{  
	FILE *fd = fopen(file,"r");  
	MD5_CTX c;  
	unsigned char md5[17]={0};  
	int i = 0;

	if(fd == NULL)  
	{  
		return -1;  
	}  
	int len;  
	unsigned char *pData = (unsigned char*)malloc(1024*1024);  
	if(!pData)  
	{       
		return -2;  
	}  
	
	MD5_Init(&c);  
	while( 0 != (len = fread(pData, 1, 1024*1024, fd) ) )  
	{  
		MD5_Update(&c, pData, len);  
	}  
	MD5_Final(md5,&c);  
		
	for(i = 0; i < 16; i++)
	{
		sprintf(buf + (i*2),"%02x",md5[i]);
	}
   
	fclose(fd);  
	free(pData);  
	return 0;  
}  
int get_string_md5(const char *src_str, char *dst_md5)  
{  
	MD5_CTX c;  
	unsigned char md5[17]={0};  
	char buf[64] = {0};
	int i = 0;
	MD5_Init(&c);  

	MD5_Update(&c, src_str, strlen(src_str));  
	
	MD5_Final(md5,&c);  
		
	for(i = 0; i < 16; i++)
	{
		sprintf(buf + (i*2),"%02x",md5[i]);
	}
	strcpy(dst_md5, buf);
	
	return 0;  
}  
int copy_file(const char* src_path, const char* dst_path)
{
	char cmd[1024] = {0};
	sprintf(cmd, "cp -fp %s %s", src_path, dst_path);
	printf("%s\n", cmd);
	system(cmd);
	return 0;
}
int delete_dir(const char* const path)
{
	
	if( access(path, F_OK) < 0)
	{
		return 0;
	}
	printf("delete  %s\n", path);
	
	return rmdir(path);
}
int delete_dir_tree(const char* const path)
{
	char cmd[1024] = {0};
	sprintf(cmd, "rm -rf %s", path);
	printf("delete tree %s\n", cmd);
	system(cmd);
	return 0;
	
}
int copy_dir(const char* src_path, const char* dst_path)
{
	char cmd[1024] = {0};
	sprintf(cmd, "cp -rpf %s %s", src_path, dst_path);
	printf("%s\n", cmd);
	system(cmd);
	return 0;
}
int check_dir(char * dir)
{
	if( access(dir, F_OK) == 0)
	{		//不纯在
		return 0;
	}
	
	if ( 0 == check_dir( dirname(dir) ) )
	{
		mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		return 0;
	}
	return -1;
}
int file_exist(const char* file)
{
	//不纯在
	return access(file, F_OK);
}
//1 目录 2 文件
int check_file_type(const char *base)
{
	struct stat ft = {0};
	
	if( lstat(base, &ft)!=0)
	{
		return -1;
	}
	if( S_ISDIR(ft.st_mode) )   ///dir
	{
		return 1;
	}
	else if(S_ISREG(ft.st_mode))
	{
		return 2;
	}
	return -2;
}

int check_file_postfix(const char * path)
{
	//.php .asp .aspx .asa .scath .bash .zsh .csh .tsch .pl .py .cgi .cfm .htaccess .jsp .txt 
	static int s_regex_flag = (REG_EXTENDED | REG_ICASE | REG_NOSUB | REG_NEWLINE);
	static char *sp_pattern_file = "(\\.txt|\\.php|\\.asp|\\.aspx|\\.asa|\\.scath|\\.bash|\\.zsh|\\.csh|\\.tsch|\\.pl|\\.py|\\.cgi|\\.cfm|\\.htaccess|\\.jsp)$";
	regex_t reg_file = {0};
    int ret = 0;	

	if(regcomp(&reg_file, sp_pattern_file, s_regex_flag)!=0)
	{
			//xm_log_write(XMLOG_WS_SCAN, LOG_ERR, __FILE__, __FUNCTION__, __LINE__, "reffind regcomp file error.");
			return -1;
	}
	ret = regexec(&reg_file, path, 8, NULL, 0);

	regfree(&reg_file);
	
	return ret;
}
int path_change_md5path(const char * src_path, char * md5_path)
{
	assert(src_path != NULL);
	assert(md5_path != NULL);
	char md5_buf[64] = {0};
	
	if(src_path == NULL || md5_path == NULL)
	{
		return -1;
	}
	get_string_md5(src_path, md5_buf);
	
	snprintf(md5_path, DIR_PATH_LEN, "%s/%s", WATCH_BAK_PATH, md5_buf);
	
	return 0;
}
//1 检测两路径是否存在包含关系
int check_child_path(const char * fi_path, const char * se_path)
{
		//判断是否是同目录下类同名文件 /var/test /var/test1
		///var/test /var/test1/tsdfss
		if(se_path == NULL || fi_path == NULL)
		{
			return -1;	
		}
		
		if( 0 == strcmp(fi_path, se_path) )
		{
			return 1;//相同	
		}
		else if( strstr(fi_path, se_path) != 0 )
		{
			if( fi_path[strlen(se_path)] == '/' )
			{
				return 1;	
			}
		}
		else if( strstr(se_path, fi_path) != 0 )
		{
			if( se_path[strlen(fi_path)] == '/' )
			{
				return 1;	
			}
		}
		return 0;
}
#ifdef TEST
int main(int argn, char ** argv)
{
	//检测是否存在如下目录，如果不纯在，那么会生成
	check_file_postfix(argv[1]);
	return 0;
}
#endif

