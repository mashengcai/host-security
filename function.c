#include "struct.h"

/* handle */
int function_1(file_s *pos)
{
	printf("%s %s\n", __FUNCTION__, pos->file_name);
	
	return 0;
}

int function_2(file_s *pos)
{
	int sockfd = g_monitor->sockfd;
	
	//send(sockfd, pos->file_name, strlen(pos->file_name), 0);

	return 0;
}
