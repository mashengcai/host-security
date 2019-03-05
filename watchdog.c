#include<sys/inotify.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

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

void *inotify_watch(void *argv){

	sleep(10);
	
	printf("add ok\n");	
       	int wd = add_wfile("./test1.txt", IN_MODIFY);

}
int main(){
	
	init_watch();
	pthread_t pthread_d[2] = {0};

	pthread_create(&pthread_d[0] , NULL, inotify_watch, NULL);

	char buf[1024] = {0};
	while(1){
		read(inotify_fd, buf, 1024);
		printf("recv\n");
	}
	return 0;;
}
