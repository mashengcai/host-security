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

#include "event_queue.h"
//#include "inotify_utils.h"
#include "file_list.h"
#include "inotify_watch.h"
#include "file_tools.h"

int keep_running;
#define INOTIFY_FLAG	(IN_MODIFY | IN_MOVE | IN_CREATE | IN_DELETE |IN_DELETE_SELF | IN_MOVE_SELF)
/* This program will take as arguments one or more directory 
   or file names, and monitor them, printing event notifications 
   to the console. It will automatically terminate if all watched
   items are deleted or unmounted. Use ctrl-C or kill to 
   terminate otherwise.
   */

static int open_inotify_fd ();
static int close_inotify_fd (int fd);
static int event_check (int fd);
static int process_inotify_events (queue_t q, int fd);
static int read_events (queue_t q, int fd);

int inotify_fd = 0;

void* inotify_watch(void *argv)
{
	/* This is the file descriptor for the inotify watch */
	int ret = 0;

	keep_running = 1;

	/* First we open the inotify dev entry */
	inotify_fd = open_inotify_fd ();
	if (inotify_fd > 0)
	{

		/* We will need a place to enqueue inotify events,
		   this is needed because if you do not read events
		   fast enough, you will miss them. This queue is 
		   probably too small if you are monitoring something
		   like a directory with a lot of files and the directory 
		   is deleted.
		   */
		queue_t q = (queue_t)argv;

		/* This is the watch descriptor returned for each item we are 
		   watching. A real application might keep these for some use 
		   in the application. This sample only makes sure that none of
		   the watch descriptors is less than 0.
		   */
		//init hash  table
		ret = hash_init_table(HASH_TABLE_SIZE);
		if(ret != 0)
		{
			printf("init hash tables error ret = %d", ret);
			exit(-1);
		}

		process_inotify_events (q, inotify_fd);
		
		/* Finish up by closing the fd, destroying the queue,
		   and returning a proper code
		   */
		close_inotify_fd (inotify_fd);
		queue_destroy (q);
	}
	return NULL;
}
int watch_file(const char *dirname)
{
	int wd;
	wd = inotify_add_watch (inotify_fd, dirname, INOTIFY_FLAG);
	if (wd < 0)
	{
		printf ("Cannot add watch for \"%s\" with event mask %lX", dirname,(unsigned long)INOTIFY_FLAG);
		fflush (stdout);
		perror (" ");
	}
	else
	{
		printf ("Watching %s WD=%d\n", dirname, wd);
		wd = hash_push(wd, dirname);
	}
	return wd;
}
int bakpath_get_filepath(char *filepath, const char * bakpath)
{
	char * bak_head_path 	= NULL;
	char * web_head_path	= NULL;
	int bak_path_len = 0;
	int ret = get_path_head_with_bak(bakpath, &bak_head_path, &web_head_path);
	if(ret != 0)
	{
		assert(0);
		return -1;
	}
	bak_path_len = strlen(bak_head_path);

	if( web_head_path[strlen(web_head_path) - 1] == '/' )
	{
		snprintf(filepath, DIR_PATH_LEN, "%s%s",web_head_path, bakpath + bak_path_len );
	}
	else if(strcmp(bakpath, bak_head_path) == 0)
	{
		snprintf(filepath, DIR_PATH_LEN, "%s",web_head_path);
	}
	else
	{
		snprintf(filepath, DIR_PATH_LEN, "%s%s",web_head_path, bakpath + bak_path_len );
	}

	return 0;
}
int rm_watch_file(const char * bak_path)
{
	int wd = 0, ret = 0;
	char web_path[DIR_PATH_LEN] = {0};
	
	bakpath_get_filepath(web_path, bak_path);
	
	wd = hash_get_key( web_path );
	
	ret = inotify_rm_watch ( inotify_fd, wd );
	
	return ret;
}
int rm_watch_tree(const char * bak_path)
{
	file_list_s * head_file_list = NULL;
	file_list_s * next_list = NULL;

	char child_path[DIR_PATH_LEN]     =   {0};
	int ret = 0, wd = 0;

	ret = get_file_list(bak_path, &head_file_list);
	if(ret != 0)
	{
		printf ("get file tree struct error bak_path =%s\n", bak_path);
		return -1;
	}

	for (next_list = head_file_list; next_list != NULL; next_list = next_list->next) 
	{		
		bakpath_get_filepath(child_path, next_list->path);
		wd = hash_get_key( child_path );
		ret = inotify_rm_watch ( inotify_fd, wd );
		if(ret != 0)
		{
			printf("error wd = %d, path = %s\n",wd, child_path);
			continue;
		}
	}
	
	free_file_list(&head_file_list);

	return 0;
}
int rm_watch_interface(const char * bak_path)
{
	int ret = 0;
	ret = check_file_type(bak_path);
	if(ret == 1)
	{
		rm_watch_tree(bak_path);
	}
	else if(ret == 2)
	{
		rm_watch_file(bak_path);
	}
	else
	{
		return -1;
	}
	return 0;
}
int watch_file_interface(const char *file_name)
{
	int file_type = 0;
	
	file_type = check_file_type(file_name);
	
	if(file_type == WATCH_DIR)
	{
		watch_tree_dir(file_name);
	}
	else if(file_type == WATCH_FILE)
	{		
		watch_file(file_name);
	}
	return 0;
}
int watch_tree_dir(const char *dir_name)
{
	file_list_s * head_file_list = NULL;
	file_list_s * next_list = NULL;
	int ret = 0 , wd = 0;

	const char * path = dir_name;

	ret = get_file_list(path, &head_file_list);
	if(ret != 0)
	{
		printf ("get file tree struct error\n");
		return -1;
	}

	for (next_list = head_file_list; next_list != NULL && wd >= 0; next_list = next_list->next) 
	{
		wd = watch_file(next_list->path);		
		if(wd < 0)
		{
			printf( "wd=%d,path=%s errno = %d\n", wd, next_list->path, errno); 
			continue;
		}
	}
	free_file_list(&head_file_list);

	return 0;
}
int open_inotify_fd ()
{
	int fd;

	//watched_items = 0;
	fd = inotify_init ();

	if (fd < 0)
	{
		perror ("inotify_init () = ");
	}
	return fd;
}


/* Close the open file descriptor that was opened with inotify_init() */
int close_inotify_fd (int fd)
{
	int r;

	if ((r = close (fd)) < 0)
	{
		perror ("close (fd) = ");
	}

	//watched_items = 0;
	return r;
}

int event_check (int fd)
{
	fd_set rfds;
	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);
	/* Wait until an event happens or we get interrupted 
	   by a signal that we catch */
	return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

int process_inotify_events (queue_t q, int fd)
{
	//while (keep_running && (watched_items > 0))
	while (keep_running)
	{
		if (event_check (fd) > 0)
		{
			int r;
			r = read_events (q, fd);
			if (r < 0)
			{
				break;
			}
			else
			{
				;//handle_events (q);
			}
		}
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

	r = read (fd, buffer, 16384);
	if (r <= 0)
		return r;
	buffer_i = 0;
	while (buffer_i < r)
	{
		/* Parse events and queue them. */
		pevent = (struct inotify_event *) &buffer[buffer_i];
		event_size =  offsetof (struct inotify_event, name) + pevent->len;
		q_event_size = offsetof (struct queue_entry, inot_ev.name) + pevent->len;
		event = malloc (q_event_size);
		memmove (&(event->inot_ev), pevent, event_size);
		queue_enqueue (event, q);
		buffer_i += event_size;
		count++;
	}
	printf ("\n%d events queued\n", count);
	return count;
}
