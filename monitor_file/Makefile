CC=gcc
#DEBUG= -DDEBUG -g
ACCERT= -DNDEBUG
CFLAGS=-Wall ${DEBUG} ${ACCERT}

HINCLUDE = -I ../../include/monitor_file -I ../../include -I ../../interface/xm_runlog_interface/include/ -I ../../interface/xm_sqlite_interface/include/

OBJS=inotify_watch.o inotify_execd.o event_queue.o file_list.o	monitor_file.o file_tools.o monitor_handle.o xm_queue.o


#LIBS= -lpthread ./libcrypto.a
#LIBS= -lpthread ../../libs/libcrypto.a
LD_RUNLOG_I = -L../../.libs -lxm_runlog_interface
LIB_PATH= -Wl,-rpath,/usr/share/xmirror/libs/xmirror

STATIC_LIB=../../libs/libcrypto.a
LIB= -lpthread $(STATIC_LIB) #-lcrypto
LIBS= $(LIB) $(LD_RUNLOG_I)

.c.o:
	$(CC) $(HINCLUDE) $(CFLAGS) -c $<

all: monitor_file

monitor_file: $(OBJS)
	$(CC) $(CFLAGS) ${OBJS} ${LIBS} ${LIB_PATH} -o monitor_file

test: hash_test tools_test

hash_test: file_list.c
	$(CC) $(CFLAGS) -DTEST file_list.c $(LIBS) -o hash_test

tools_test: file_tools.c
	$(CC) $(CFLAGS) -DTEST file_tools.c $(LIBS) -o tools_test

clean:
	-rm -f $(OBJS) *.bak *.o
	-rm -rf monitor_file
	-rm -rf hash_test
	
