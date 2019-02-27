#include<sys/inotify.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <hlist.h>

#ifdef TEST
	#include<stdio.h>
#endif

struct file_wd{
	int wd;	
	char *filename;
	struct list *node;
};

static int inotify_fd = 0;

void init_watch()
{
	if( 0 == inotify_fd )
		inotify_fd = inotify_init();

	if(inotify_init < 0){
		perror(__FUNCTION__);	
		exit(errno);
	}
}
void free_watch()
{
	if( 0 != inotify_fd )
		close(inotify_fd);

}

int add_wfile(const char *filename, unsigned int flag)
{
	if(filename == NULL)
		return -1;

	
	int wd = inotify_add_watch(inotify_fd, filename, flag);
	if(-1 == wd)
		return wd;

	
	
	
}

int del_wfile(const char *filename)
{
	int wd = find_fd(filename);

	if(wd){
		inotify_rm_watch(inotify_fd, wd);
	}
	else{
		/*del error*/
		return -1;
	}

	return 0;
}



#ifdef TEST
int main(){
	
	init_watch();

       	int wd1 = watch_file("./test1.txt", IN_MODIFY);

       	int wd2 = watch_file("./test2.txt", IN_MODIFY);

	
	char buf[1024] = {0};
	while(1){
		read(inotify_fd, buf, 1024);
		printf("recv\n");
	}

	inotify_rm_watch(inotify_fd, wd1);
	inotify_rm_watch(inotify_fd, wd2);
		
	free_watch();

	return 0;;
}
#endif
