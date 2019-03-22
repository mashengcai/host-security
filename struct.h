#pragma once

#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF)

#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF)

#define EVENT_LOCK	pthread_mutex_lock(&gs_list_mtx);
#define EVENT_UNLOCK	pthread_mutex_unlock(&gs_list_mtx);

typedef struct file_list
{
	char *file_name;
	int wd;
	struct list_head node;
}file_s;

typedef struct queue_t
{
	struct list_head 		node;
	struct inotify_event 	inot_ev;
}queue_s;

typedef struct monitor{
	int inotify_fd;
	int keep_running;
	struct list_head queue;
	struct list_head file_list;
}monitor_s;
