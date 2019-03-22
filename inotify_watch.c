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

#include "struct.h"

static int event_check (int fd);
static int process_inotify_events (struct list_head *q, int fd);
static int read_events (queue_t q, int fd);

void* inotify_watch(void *argv)
{
	int ret = 0;
	
	monitor_s * monitor_t = (monitor_s *)argv;
	
	watch_file_interface(monitor_t->file_list);
	
	process_inotify_events (&monitor_t->queue, monitor_t->inotify_fd);
	
	return NULL;
}

int watch_file(const char *dirname)
{
	int wd = inotify_add_watch (inotify_fd, dirname, INOTIFY_FLAG);
	
	if (wd < 0)
		debuginfo(LOG_ERROR, "Watching %s WD=%d\n", dirname, wd);
	else
		debuginfo(LOG_DEBUG, "Watching %s WD=%d\n", dirname, wd);
	
	return wd;
}

int rm_watch_file(const char * path)
{
	file_s *pos = NULL；
	
	list_for_each_entry(pos, head, node){
		if( !strcmp(path, post->file_name )){
			if(0 == inotify_rm_watch ( inotify_fd, wd)){
				debuginfo(LOG_DEBUG, "del Watch %s WD=%d\n", dirname, wd);
				return 0;
			}		
		}
	}
	
	debuginfo(LOG_ERROR, "del Watch %s WD=%d\n", dirname, wd);
	return -1;
}

int rm_watch_interface(const char * path)
{
	return rm_watch_file(path);
}

int watch_file_interface(struct list_head *list)
{
	int file_type = 0;
	file_s *pos = NULL;
	int wd = 0;
	
	list_for_each_entry(pos, list, node){
		//file_type = check_file_type(pos->file_name);
		 wd = watch_file(pos->file_name);
		 if(wd <= 0)
			 pos->wd = 0;
		 pos->wd = wd;
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

int process_inotify_events (struct list_head *q, int fd)
{
	while (keep_running)
	{
		if (event_check (fd) > 0)
			if (read_events (q, fd) < 0)
				break;
	}
	return 0;
}

int read_events (struct list_head *q, int fd)
{
	char buffer[16384];
	size_t buffer_i;
	struct inotify_event *pevent;
	queue_s *event;
	ssize_t r;
	size_t event_size, q_event_size;
	int count = 0;

	r = read (fd, buffer, 16384);
	if (r <= 0)
		return r;
	buffer_i = 0;
	while (buffer_i < r)
	{
		/* Parse events and queue them. */
		pevent = (struct inotify_event *) &buffer[buffer_i];
		event_size =  offsetof (struct inotify_event, name) + pevent->len;
		q_event_size = offsetof (struct queue_t, inot_ev.name) + pevent->len;
		event = MALLOC (q_event_size);
		memmove (&(event->inot_ev), pevent, event_size);
		list_add(&event->node, q);
		buffer_i += event_size;
		count++;
	}
	return count;
}
