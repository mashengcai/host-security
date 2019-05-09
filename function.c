#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "cjson.h"
#include "struct.h"

char *struct_2_json(handle_s *pos){

	cjson *root = NULL;

	root = create_obj();

	add_str_2_obj(root, "dir_name", 	pos->dir_name);
	add_str_2_obj(root, "file_name", 	pos->file_name);
	add_str_2_obj(root, "mask", 		pos->mask);

	add_int_2_obj(root, "time", 		pos->time);
	add_int_2_obj(root, "WD", 		pos->wd);

	return cJSON_Print(root);
}

/* handle */
int function_1(handle_s *pos)
{
	printf("%s %s\n", __FUNCTION__, struct_2_json(pos));
	
	return 0;
}

int function_2(handle_s *pos)
{
	int sockfd = g_monitor->sockfd;

	char *out = struct_2_json(pos);

	if(out != NULL)
		send(sockfd, out, strlen(out), 0);

	return 0;
}
