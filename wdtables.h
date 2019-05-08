#ifndef __WD_TABLES__
#define __WD_TABLES__

#include <hlist.h>

typedef struct wd_maps{
	int wd;
	char *dir_name;
	char *file_name;
	struct hlist_node h_node;
}wdtable_s;

#define WD_MAPS_SIZE (sizeof(wdtable_s))
#define HASH_NUM 	0xFF

int push_wd(int wd, const char *file_name, const char *dir_name);

wdtable_s *wd_2_path(int wd);

int free_wd(int wd);

#endif
