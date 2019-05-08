#include <stdio.h>
#include <string.h>
#include <hlist.h>
#include <stdlib.h>

#include "wdtables.h"

#if DEBUG
	static unsigned int mac_num = 0;

	void *mmalloc(size_t size){
		mac_num++;
		return malloc(size);
	}

	void ffree(void *p){
		mac_num--;
		return free(p);
	}

	#define	ECHO(n) printf("mac_num = %u\n", n)

	#define MALLOC		mmalloc 
	#define FREE		ffree
#else
	#define MALLOC		malloc 
	#define FREE		free
#endif

static struct hlist_head wd_tables[HASH_NUM] = {{0}};

static unsigned int hash_fun(unsigned int wd)
{
	return wd & HASH_NUM;
}

int push_wd(int wd, const char *file_name, const char *dir_name)
{
	if(file_name == NULL)
		return -1;

	wdtable_s *pos = MALLOC(WD_MAPS_SIZE);
	memset(pos, 0, WD_MAPS_SIZE);

	pos->wd = wd;

	if(dir_name != NULL){
		pos->dir_name = MALLOC(strlen(dir_name + 1));
		strcpy(pos->dir_name, dir_name);
	}

	if(file_name != NULL){
		pos->file_name = MALLOC(strlen(file_name + 1));
		strcpy(pos->file_name, file_name);
	}
	
	hlist_add_head(&pos->h_node, &wd_tables[hash_fun(wd)]);

	return 0;
}

wdtable_s *wd_2_path(int wd)
{
	wdtable_s *tpos = NULL;
	struct hlist_node *pos = NULL;

	unsigned int key = hash_fun(wd);
	
	hlist_for_each_entry(tpos, pos, &wd_tables[key], h_node){
		if(tpos->wd == wd)
			return tpos;
	}

	return NULL;
}

int free_wd(int wd)
{
	wdtable_s *node = wd_2_path(wd);
	if(node == NULL)
		return -1;

	hlist_del(&node->h_node);

	if(node->dir_name != NULL)
		FREE(node->dir_name);

	if(node->file_name != NULL)
		FREE(node->file_name);
	
	FREE(node);

	return 0;
}
#if DEBUG
int main()
{
	int n = 50, i = 0;
	char buf[128] = {0};

	// add 
	for(; i < n; i++){
		sprintf(buf, "filename = %d_.txt", i); 
		push_wd(i, buf, NULL);
	}   
	printf("add OK!!!\n");

	ECHO(mac_num);

	//list
	for(i = 0; i < n; i++){
		wdtable_s *pos = wd_2_path(i);
		if(pos)
			printf("%d = %s\n", i, pos->file_name);
	}

	// del
	
	for(i = 0; i < n; i++)
		free_wd(i);

	ECHO(mac_num);

	//list
	for(i = 0; i < n; i++){
		wdtable_s *pos = wd_2_path(i);
		if(pos)
			printf("%d = %s\n", i, pos->file_name);
	}

	return 0;
}
#endif
