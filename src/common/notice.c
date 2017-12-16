#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>

#include "define.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int notice_create(int offset)
{
    int sid;
    key_t key;
    union semun un_sem;

    memset(&un_sem, 0, sizeof(un_sem));
    key = ftok(FTOK_FILE, offset);
    if ((key_t)-1 == key) {
        err("ftok error [%d][%d]", getpid(), errno);
        return -1;
    }

    sid = semget(key, 1, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR);
    if (-1 == sid) {
        err("semget error [%d][%x][%d]", getpid(), key, errno);
        return -1;
    }

    info("notice_create ok [%d][%x][%d]", getpid(), key, sid);

    return sid;
}

int notice_open(int offset)
{
    int sid;
    key_t key;
    union semun un_sem;

    memset(&un_sem, 0, sizeof(un_sem));
    key = ftok(FTOK_FILE, offset);
    if ((key_t)-1 == key) {
        err("ftok error [%d][%d]", getpid(), errno);
        return -1;
    }

    sid = semget(key, 1, IPC_CREAT|S_IRUSR|S_IWUSR);
    if (-1 == sid) {
        err("semget error [%d][%x][%d]", getpid(), key, errno);
        return -1;
    }

    info("notice_open ok [%d][%x][%d]", getpid(), key, sid);

    return sid;
}

int notice_fire(int sid)
{
    union semun un_sem;

    memset(&un_sem, 0, sizeof(un_sem));
    un_sem.val = 1;

    if (-1 == semctl(sid, 0, SETVAL, un_sem)) {
        err("semctl error [%d][%d][%d]", getpid(), sid, errno);
        return -1;
    }

    return 0;
}

int notice_wait(int sid)
{
    struct sembuf st_sem;

    memset(&st_sem, 0, sizeof(st_sem));
    st_sem.sem_num = 0;
    st_sem.sem_op = -1;
    st_sem.sem_flg = SEM_UNDO;

    if (-1 == semop(sid, &st_sem, 1)) {
        err("semop error [%d][%d][%d]", getpid(), sid, errno);
        return -1;
    }

    return 0;
}

int notice_destroy(int sid)
{
    union semun un_sem;

    memset(&un_sem, 0, sizeof(un_sem));
    if (-1 == semctl(sid, 0, IPC_RMID, un_sem)) {
        err("semctl error [%d][%d][%d]", getpid(), sid, errno);
        return -1;
    }

    return 0;
}
