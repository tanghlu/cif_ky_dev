
INCLUDE=-I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0
LIBS=/usr/local/lib/libmongoc-1.0.so /usr/local/lib/libbson-1.0.so

test: test.c
	gcc -o $@ $+ ${INCLUDE} ${LIBS}
