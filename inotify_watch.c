#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <signal.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <libgen.h>
#include <assert.h>
#include <dirent.h>

#include "struct.h"
#include "runlog.h"
#include "wdtables.h"
#include "file_tools.h"

static int event_check (int fd);
static int process_inotify_events (queue_t *q, int fd);
static int read_events (queue_t q, int fd);
static int watch_file_interface(struct list_head *list);

int *inotify_fp = 0;

void* inotify_watch(void *argv)
{
	monitor_s * monitor_t = (monitor_s *)argv;
		
	inotify_fp = &monitor_t->inotify_fd;

	watch_file_interface(&monitor_t->file_list);
	
	process_inotify_events (&monitor_t->queue, monitor_t->inotify_fd);
	
	return NULL;
}

int watch_file(const char *dirname)
{
	int wd = inotify_add_watch (*inotify_fp, dirname, INOTIFY_FLAG);
	
	if (wd < 0)
		debuginfo(LOG_ERROR, "Watching %s WD=%d", dirname, wd);
	else
		debuginfo(LOG_DEBUG, "Watching %s WD=%d", dirname, wd);
	
	return wd;
}

int watch_tree_dir(const char *path, const char *root)
{
	DIR *dir;
	struct dirent *ptr;
	struct stat ft = {0};

	char base[1024];
	assert( path != NULL );
	
	if(path == NULL || path[0] == '\0')
		return -1;
	

	if ((dir = opendir(path)) == NULL){
		perror("Open dir error...");
		return -2;
	}

	while ((ptr = readdir(dir)) != NULL){
		if( strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name, "..") == 0 ) 
			continue;

		memset(base, 0, sizeof(base));

#if 0
		if(path[strlen(path) - 1] == '/')
			sprintf(base, "%s%s",path, ptr->d_name);
		else
			sprintf(base, "%s/%s",path, ptr->d_name);
#else
		sprintf(base, "%s/%s",path, ptr->d_name);
#endif

		if( lstat(base, &ft)!= 0)
			continue;

		if( S_ISDIR(ft.st_mode) ){
			int wd = watch_tree_dir(base, root);
			if(wd > 0)
				push_wd(wd, base, root);
		}
	}
	closedir(dir);

	return watch_file(path);
}

int rm_watch_file(struct list_head *head, const char * path)
{
	file_s *pos = NULL;
	
	list_for_each_entry(pos, head, node){
		if( !strcmp(path, pos->file_name )){
			if(0 == inotify_rm_watch ( *inotify_fp, pos->wd)){
				debuginfo(LOG_DEBUG, "del Watch %s WD=%d", dirname, pos->wd);
				pos->wd = 0;
				return 0;
			}		
		}
	}
	
	debuginfo(LOG_ERROR, "del Watch %s WD=%d", dirname, pos->wd);
	return -1;
}

int rm_watch_interface(struct list_head *head, const char * path)
{
	return rm_watch_file(head, path);
}

int watch_file_interface(struct list_head *list)
{
	file_s *pos = NULL;
	int wd = 0;
	
	list_for_each_entry(pos, list, node){
		if(pos->file_type == FILE_TYPE){
			wd = watch_file(pos->file_name);
		}else if(pos->file_type == DIR_TYPE){
			wd = watch_tree_dir(pos->file_name, pos->file_name);
		}
		if(wd > 0)
			push_wd(wd, pos->file_name, NULL);
	}

	return 0;
}

int event_check (int fd)
{
	fd_set rfds;
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);

	return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

int process_inotify_events (queue_t *q, int fd)
{
	while(list_entry(q, struct monitor, queue)->keep_running)
	{
		if (event_check (fd) > 0)
			if (read_events (*q, fd) < 0)
				break;
	}
	return 0;
}

int read_events (queue_t q, int fd)
{
	char buffer[16384];
	size_t buffer_i;
	struct inotify_event *pevent;
	queue_entry_t event;
	ssize_t r;
	size_t event_size, q_event_size;
	int count = 0;

	memset(buffer, 0, sizeof(buffer));
	r = read (fd, buffer, 16384);
	if (r <= 0)
		return r;
	buffer_i = 0;
	while (buffer_i < r){
		pevent = (struct inotify_event *) &buffer[buffer_i];
		event_size =  offsetof (struct inotify_event, name) + pevent->len;
		q_event_size = offsetof (struct queue_entry, inot_ev.name) + pevent->len;
		event = MALLOC (q_event_size);
		memmove (&(event->inot_ev), pevent, event_size);
		queue_enqueue (event, q);
		buffer_i += event_size;
		count++;

		debuginfo(LOG_DEBUG, "events wd = %d count = %d", pevent->wd, count);
	}
	return count;
}

