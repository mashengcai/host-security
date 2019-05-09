#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <assert.h>
#include <errno.h>
#include <sys/inotify.h>

#include <runlog.h>
#include "struct.h"
#include "wdtables.h"

char *analysis_event(unsigned int mask)
{	
	char *flag = NULL;

	switch (mask & (IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED)){
			/* File was accessed */
		case IN_ACCESS:
			flag = "IN_ACCESS";
			break;

			/* File was modified */
		case IN_MODIFY:
			flag = "IN_MONDIFY";
			break;

			/* File changed attributes */
		case IN_ATTRIB:
			flag = "IN_ATTRIB";
			break;

			/* File open for writing was closed */
		case IN_CLOSE_WRITE:
			flag = "IN_CLOSE_WRITE";
			break;

			/* File open read-only was closed */
		case IN_CLOSE_NOWRITE:
			flag = "IN_CLOSE_NOWRITE";
			break;

			/* File was opened */
		case IN_OPEN:
			flag = "IN_OPEN";
			break;

			/* File was moved from X */
		case IN_MOVED_FROM:
			flag = "IN_MOVED_FROM";
			break;

			/* File was moved to X */
		case IN_MOVED_TO:
			flag = "IN_MOVED_TO";
			break;

			/* Subdir or file was deleted */
		case IN_DELETE:
			flag = "IN_DELETE";
			break;

			/* Subdir or file was created */
		case IN_CREATE:
			flag = "IN_CREATE";
			break;

			/* Watched entry was deleted */
		case IN_DELETE_SELF:
			flag = "IN_DELETE_SELF";
			break;

			/* Watched entry was moved */
		case IN_MOVE_SELF:
			flag = "IN_MOVE_SELF";
			break;

			/* Backing FS was unmounted */
		case IN_UNMOUNT:
			flag = "IN_UNMOUNT";
			break;

			/* Too many FS events were received without reading them
			   some event notifications were potentially lost.  */
		case IN_Q_OVERFLOW:
			flag = "IN_Q_OVERLOW";
			break;

			/* Watch was removed explicitly by inotify_rm_watch or automatically
			   because file was deleted, or file system was unmounted.  */
		case IN_IGNORED:
			flag = "IN_IGNORED";
			break;

			/* Some unknown message received */
		default:
			flag = "UNKNOWN_EVENT";
			break;
	}

	return flag;
}

void handle_debug_ex (struct list_head *head, queue_entry_t event)
{
	/* If the event was associated with a filename, we will store it here */
	char *cur_event_filename = NULL;
	char *cur_event_file_or_dir = NULL;
	/* This is the watch descriptor the event occurred on */
	int cur_event_wd = event->inot_ev.wd;
	int cur_event_cookie = event->inot_ev.cookie;
	unsigned long flags;
	char buf[1024] = {0};
	wdtable_s *wdtab = NULL;

	wdtab = wd_2_path(cur_event_wd);

	if(wdtab != NULL){
		cur_event_filename = wdtab->file_name;
	}
	
	if (event->inot_ev.len){
		sprintf(buf, "%s/%s", cur_event_filename, event->inot_ev.name);
		cur_event_filename = buf;
	}

	flags = event->inot_ev.mask & 
		~(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED );

	if ( event->inot_ev.mask & IN_ISDIR ){
		cur_event_file_or_dir = "Dir";
	}
	else {
		cur_event_file_or_dir = "File";
	}
	
	switch (event->inot_ev.mask & 
			(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED))
	{
		/* File was accessed */
		case IN_ACCESS:
			printf ("ACCESS: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File was modified */
		case IN_MODIFY:
			printf ("MODIFY: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File changed attributes */
		case IN_ATTRIB:
			printf ("ATTRIB: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File open for writing was closed */
		case IN_CLOSE_WRITE:
			printf ("CLOSE_WRITE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File open read-only was closed */
		case IN_CLOSE_NOWRITE:
			printf ("CLOSE_NOWRITE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File was opened */
		case IN_OPEN:
			printf ("OPEN: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* File was moved from X */
		case IN_MOVED_FROM:
			printf ("MOVED_FROM: %s \"%s\" on WD #%i. Cookie=%d\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd, 
					cur_event_cookie);
			break;

			/* File was moved to X */
		case IN_MOVED_TO:
			printf ("MOVED_TO: %s \"%s\" on WD #%i. Cookie=%d\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd, 
					cur_event_cookie);
			break;

			/* Subdir or file was deleted */
		case IN_DELETE:
			printf ("DELETE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Subdir or file was created */
		case IN_CREATE:
			printf ("CREATE: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Watched entry was deleted */
		case IN_DELETE_SELF:
			printf ("DELETE_SELF: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Watched entry was moved */
		case IN_MOVE_SELF:
			printf ("MOVE_SELF: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Backing FS was unmounted */
		case IN_UNMOUNT:
			printf ("UNMOUNT: %s \"%s\" on WD #%i\n",
					cur_event_file_or_dir, cur_event_filename, cur_event_wd);
			break;

			/* Too many FS events were received without reading them
			   some event notifications were potentially lost.  */
		case IN_Q_OVERFLOW:
			printf ("Warning: AN OVERFLOW EVENT OCCURRED: \n");
			break;

			/* Watch was removed explicitly by inotify_rm_watch or automatically
			   because file was deleted, or file system was unmounted.  */
		case IN_IGNORED:
			/*watched_items--;*/
			printf ("IGNORED: WD #%d\n", cur_event_wd);
			/* printf("Watching = %d items\n",watched_items); */
			break;

			/* Some unknown message received */
		default:
			printf ("UNKNOWN EVENT \"%X\" OCCURRED for file \"%s\" on WD #%i\n",
					event->inot_ev.mask, cur_event_filename, cur_event_wd);
			break;
	}
	/* If any flags were set other than IN_ISDIR, report the flags */
	if (flags & (~IN_ISDIR)){
		flags = event->inot_ev.mask;
		printf ("Flags=%lX\n", flags);
	}
}

file_s *path_2_files(struct list_head *head, wdtable_s *tab)
{
	char *path = NULL;

	if(tab->dir_name != NULL)
		path = tab->dir_name;
	else
		path = tab->file_name;

	if(path == NULL)
		return NULL;

	file_s *pos = NULL;

	list_for_each_entry(pos, head, node){
		if( 0 == strcmp(pos->file_name, path))
			return pos;
	}

	return NULL;
}

void handle_event (struct list_head *head, queue_entry_t event)
{

	//---------debug info--------------
	//handle_debug_ex(head, event);
	
	int i = 0, ret = 0;
	int cur_event_wd = event->inot_ev.wd;
	file_s *pos = NULL;
	wdtable_s *wtab = NULL;
	handle_s data = {0};
	char buf[512] = {0};
	
	wtab = wd_2_path(cur_event_wd);
	if(wtab == NULL){
		debuginfo(LOG_ERROR, "get path error wd = %d", cur_event_wd);
		return;
	}

	pos = path_2_files(head, wtab);
	if(pos == NULL){
		debuginfo(LOG_ERROR, "get file error file_name = %d", wtab->dir_name);
		return;
	}

	/* handle */
	if (event->inot_ev.len){
		sprintf(buf, "%s/%s", wtab->file_name, event->inot_ev.name);
		data.file_name = buf;
	}
	
	if(wtab->dir_name == NULL)
		data.dir_name = wtab->file_name;
	else
		data.dir_name = wtab->dir_name;

	data.wd = cur_event_wd;
	data.mask = analysis_event(event->inot_ev.mask);
	
	for(; i < pos->f_num; i++){
		ret = pos->f_arr[i](&data);
		if(ret < 0)
			debuginfo(LOG_ERROR, "path=%s,wd=%d,ret=%d\n", pos->file_name, cur_event_wd, ret);
		debuginfo(LOG_DEBUG, "path=%s,wd=%d,ret=%d\n", pos->file_name, cur_event_wd, ret);
	}
	
	return ;
}

void handle_events (struct list_head *head, queue_t q)
{
	queue_entry_t event;
	while (!queue_empty (q)){
		event = queue_dequeue (q);
		handle_event (head, event);
		free (event);
	}
}

void *inotify_execd(void *argv)
{
	monitor_s *monitor_t = (monitor_s *)argv;

	while(1){
		handle_events(&monitor_t->file_list, monitor_t->queue);
		usleep(100000);
	}

	return NULL;
}

