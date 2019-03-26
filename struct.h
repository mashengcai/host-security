#pragma once

#include <hlist.h>
#include "event_queue.h"

#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF)

#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF)

#define EVENT_LOCK	pthread_mutex_lock(&gs_list_mtx);
#define EVENT_UNLOCK	pthread_mutex_unlock(&gs_list_mtx);

#define MALLOC	malloc

typedef struct file_list
{
	char *file_name;
	int wd;
	struct list_head node;
}file_s;

typedef struct monitor{
	int 		inotify_fd;
	int 		keep_running;
	struct queue_struct *queue;
	struct list_head file_list;
}monitor_s;
