#pragma once

#include <stdio.h>
#include <stdarg.h>

#define MSG_DATA_LEN	4096

#define ERROR 	"error", __FILE__, __LINE__, __FUNCTION__
#define DEBUG 	"debug", __FILE__, __LINE__, __FUNCTION__


int runlog_open();
int debuginfo(const char *type, char * file, int line, const char * fun, char *format, ...);
void runlog_close();
