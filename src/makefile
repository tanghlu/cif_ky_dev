
INCLUDE=-I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 -I${HOME}/ecif/include
LIBS=/usr/local/lib/libmongoc-1.0.so /usr/local/lib/libbson-1.0.so

connector: connector.c common/log.c common/notice.c
	gcc -o $@ $+ ${INCLUDE} ${LIBS}
