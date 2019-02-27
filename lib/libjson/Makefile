all: libcjson.so

libcjson.so: cJSON.c cjson.c
	gcc -g -shared cJSON.c cjson.c -o libcjson.so -fPIC -Wall

test:libcjson.so
	gcc -DDEBUG cjson.c -o test -L./ -lcjson -lm -Wl,-rpath=./

clean:
	-rm -f libcjson.so
	-rm -f test
