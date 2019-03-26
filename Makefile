CC=gcc
CFLAG=-g -Wall

INCS=-I./lib/ -I./lib/librunlog -I./lib/libsda

LIBS=-L ./lib/librunlog/ -lrunlog  -L./lib/libsda/ -lcomm  -lpthread

OTHER=-Wl,-rpath=./lib/librunlog/ -Wl,-rpath=./lib/libsda/

RUNLIB=./lib/librunlog/librunlog.so

all:host_security

OBJS=inotify_watch.o inotify_execd.o main.o event_queue.o

host_security: $(OBJS) $(RUNLIB)
	$(CC) $(INCS) $(CFLAG) $(OBJS) $(LIBS) $(OTHER) -o host_security

$(OBJS):%.o:%.c
	$(CC) $(INCS) $(CFLAG) -c -o $@ $<

$(RUNLIB):
	make -C lib/


clean:
	$(RM) $(OBJS)
	$(RM) host_security
	make clean -C lib
	$(RM) host_security
