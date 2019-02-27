#pragma once

#include <stdio.h>
#include <stdarg.h>

#define MSG_DATA_LEN	4096

#define SYS_TYPE "SYS_TYPE"
#define ENAB_TYPE "ENAB_TYPE"
#define MEMDUMP_TYPE "MEMDUMP_TYPE"
#define CFINFO_TYPE "CFINFO_TYPE"
#define PROC_TYPE "PROC_TYPE"
#define FILE_TYPE "FILE_TYPE"



#define SYS_FORMAT 	SYS_TYPE, __FILE__, __LINE__, __FUNCTION__
#define ENAB_FORMAT ENAB_TYPE, __FILE__, __LINE__, __FUNCTION__
#define MEMDUMP_FORMAT MEMDUMP_TYPE, __FILE__, __LINE__, __FUNCTION__
#define CFINFO_FORMAT CFINFO_TYPE, __FILE__, __LINE__, __FUNCTION__
#define PROC_FORMAT PROC_TYPE, __FILE__, __LINE__, __FUNCTION__
#define FILE_FORMAT FILE_TYPE, __FILE__, __LINE__, __FUNCTION__



#define SYS_FORMAT_ERR  	SYS_FORMAT
#define ENAB_FORMAT_ERR  	ENAB_FORMAT
#define MEMDUMP_ERR  		MEMDUMP_FORMAT
#define CFINFO_ERR  		CFINFO_FORMAT
#define PROC_ERR  			PROC_FORMAT
#define FILE_ERR  			FILE_FORMAT


#define ENAB_FORMAT_INFO 	ENAB_FORMAT
#define SYS_FORMAT_INFO 	SYS_FORMAT
#define MEMDUMP_INFO 		SYS_FORMAT
#define CFINFO_INFO 		CFINFO_FORMAT
#define PROC_INFO 			PROC_FORMAT
#define FILE_INFO 			FILE_FORMAT


int runlog_open();
int debuginfo(const char *type, char * file, int line, const char * fun, char *format, ...);
void runlog_close();
