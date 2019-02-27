all: libcomm.so

CC=gcc
CFLAG=-Wall -O2

SOURCE=filecomm.c stdcomm.c

libcomm.so: $(SOURCE)
	$(CC) $(CFLAG) -shared $(SOURCE) -o libcomm.so -fPIC -Wall

clean:
	-rm -f libcomm.so
