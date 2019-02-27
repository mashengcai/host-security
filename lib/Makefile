TARGET=librunlog.so libsda.so libcjson.so

all:$(TARGET)

librunlog.so:
	make -C librunlog

libsda.so:
	make -C libsda

libcjson.so:
	make -C libjson


clean:
	make clean -C librunlog
	make clean -C libsda
	make clean -C libjson

distclean:clean
