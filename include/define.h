#ifndef _DEFINE_H_
#define _DEFINE_H_

#define LISN_PORT 8899
#define HEAD_FOR_LEN 4
#define FTOK_FILE "/tmp"

#define SEMID_PROCESSOR 1
#define SEMID_TIMER 2

#define err(fmt, ...) write_log(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define info(fmt, ...) write_log(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define trace(fmt, ...) write_log(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
