TARGET=librunlog.so libstd.so libcjson.so

all:$(TARGET)

librunlog.so:
	make -C librunlog

libstd.so:
	make -C libstd

libcjson.so:
	make -C libjson


clean:
	make clean -C librunlog
	make clean -C libstd
	make clean -C libjson

distclean:clean
