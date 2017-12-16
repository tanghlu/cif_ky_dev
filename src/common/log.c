#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#define PATH_MAX 255

void write_log(char* file, int line, const char* fmt, ...)
{
    FILE *fp;
    time_t ctm;
    struct tm *stm;
    char filename[PATH_MAX];
    va_list args;

    time(&ctm);
    stm=localtime(&ctm);

    sprintf(filename, "%s/log%d%02d%02d", getenv("HOME"), stm->tm_year+1900, stm->tm_mon+1, stm->tm_mday);

    va_start(args, fmt);
    fp = fopen(filename, "a+");
    if (fp == NULL) return;

    fprintf(fp, "\n%02d:%02d:%02d|%05d|:%s:%d|",
                stm->tm_hour, stm->tm_min, stm->tm_sec,
                getpid(), file, line);
    fflush(fp);
    vfprintf(fp, fmt, args);
    va_end(args);

    fclose(fp);
}
