#pragma once

#include "runlog.h"
#include "stdcomm.h"
#include "event_queue.h"
#include "hlist.h"

#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF|IN_ISDIR)

#define EVENT_LOCK	pthread_mutex_lock(&gs_list_mtx);
#define EVENT_UNLOCK	pthread_mutex_unlock(&gs_list_mtx);

#define MALLOC	malloc

#define FUN_MAX 32

typedef struct file_list
{
	char *file_name;
	unsigned int action;
	int wd;
	struct list_head node;
		
	int f_num;
	int (*f_arr[FUN_MAX])(struct file_list *);
}file_s;

typedef struct monitor{
	int 		inotify_fd;
	int 		keep_running;
	struct queue_struct *queue;
	struct list_head file_list;
}monitor_s;
