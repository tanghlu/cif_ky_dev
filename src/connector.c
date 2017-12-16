#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mongoc.h>

#include "define.h"

static int process(int s);
static int db_put_in(const char *in);
static char* db_get_out(int *len);

int main(int argc, char *argv[])
{
    int s_lisn = -1, s_conn = -1;
    struct sockaddr_in sa_bind, sa_acpt;
    int reuse = 1;
    pid_t pid;
    int len = 0, rtn = 0;

    signal(SIGCHLD, SIG_IGN);

    memset(&sa_bind, 0, sizeof(sa_bind));
    sa_bind.sin_addr.s_addr = htonl(INADDR_ANY);
    sa_bind.sin_family = AF_INET;
    sa_bind.sin_port = htons(LISN_PORT);

    s_lisn = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == s_lisn) {
        err("socket error [%d]\n", errno);
        return -1;
    }

    setsockopt(s_lisn, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    rtn = bind(s_lisn, (struct sockaddr *)&sa_bind, sizeof(sa_bind));
    if (-1 == rtn) {
        err("bind error [%d]\n", errno);
        close(s_lisn);
        return -1;
    }

    rtn = listen(s_lisn, SOMAXCONN);
    if (-1 == rtn) {
        err("listen error [%d]\n", errno);
        close(s_lisn);
        return -1;
    }

    for ( ; ; ) {
        memset(&sa_acpt, 0, sizeof(sa_acpt));
        s_conn = accept(s_lisn, (struct sockaddr *)&sa_acpt, &len);
        if (-1 == s_conn) {
            err("accept error [%d]\n", errno);
            close(s_lisn);
            return -1;
        }
        info("accept ok [%d]\n", s_conn);

        pid = fork();
        if (0 > pid) {
            close(s_conn);
            close(s_lisn);
            exit(-1);
        } else if (0 < pid) {
            close(s_conn);
        } else {
            close(s_lisn);

            process(s_conn);

            close(s_conn);

            exit(0);
        }
    }

    return 0;
}

static int process(int s)
{
    int sem_connector, sem_processor, sem_timer;
    char buffer[HEAD_FOR_LEN+1];
    char *in, *out;
    int len, rtn;

    sem_connector = notice_create(getpid());
    sem_processor = notice_open(SEMID_PROCESSOR);
    sem_timer = notice_open(SEMID_TIMER);

    mongoc_init();

    memset(buffer, 0, sizeof(buffer));
    rtn = recv(s, buffer, HEAD_FOR_LEN, 0);
    if (HEAD_FOR_LEN != rtn) {
        err("recv error [%d]\n", errno);
        rtn = -1;
        goto cleanup;
    }

    len = atoi(buffer);
    in = calloc(sizeof(char), len+1);
    rtn = recv(s, in, len, 0);
    if (len != rtn) {
        err("recv error [%d]\n", errno);
        rtn = -1;
        goto cleanup;
    }

    info("recv ok [%d]\n", len);

    db_put_in(in);

    notice_fire(sem_processor);
    notice_fire(sem_timer);

    notice_wait(sem_connector);

    out = db_get_out(&len);
    if (NULL == out) {
        err("db_get_out error [%d]\n", errno);
        rtn = -1;
        goto cleanup;
    }

    snprintf(buffer, HEAD_FOR_LEN, "%d", len);
    rtn = send(s, buffer, HEAD_FOR_LEN, 0);
    if (HEAD_FOR_LEN != rtn) {
        err("send error [%d]\n", errno);
        rtn = -1;
        goto cleanup;
    }
    rtn = send(s, out, len, 0);
    if (len != rtn) {
        err("send error [%d]\n", errno);
        rtn = -1;
        goto cleanup;
    }

    info("send ok [%d]\n", len);
    rtn = 0;

cleanup:
    free(in);
    free(out);

    notice_destroy(sem_connector);
    mongoc_cleanup();

    return rtn;
}

static int db_put_in(const char *in)
{
    const char *uri = "mongodb://127.0.0.1:27017";
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_t *insert, reply;
    bson_error_t error;
    int rtn;

    client = mongoc_client_new(uri);

    mongoc_client_set_appname(client, "connector");

    collection = mongoc_client_get_collection(client, "test", "tbl1");

    insert = BCON_NEW("pid", BCON_INT32(getpid()), "in", BCON_UTF8(in));

    if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, insert, NULL, &error)) {
        err("mongoc_collection_insert error [%s]", error.message);
        rtn = -1;
        goto cleanup;
    }
    rtn = 0;

cleanup:
    bson_destroy(insert);
    bson_destroy(&reply);

    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return rtn;
}

static char* db_get_out(int *len)
{
    const char *uri = "mongodb://127.0.0.1:27017";
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    const bson_t *doc;
    bson_t *query, *update, reply;
    mongoc_cursor_t *cursor;
    bson_error_t error;
    int rtn;

    client = mongoc_client_new(uri);

    mongoc_client_set_appname(client, "connector");

    collection = mongoc_client_get_collection(client, "test", "tbl1");

    query = BCON_NEW("pid", BCON_INT32(getpid()));
    update = BCON_NEW("$set", "{", "pid", BCON_INT32(0), "}");

    cursor = mongoc_collection_find_with_opts(collection, (const bson_t *)&query, NULL, NULL);

    while (mongoc_cursor_next(cursor, &doc)) {
        ;
    }

    if (mongoc_cursor_error(cursor, &error)) {
        err("mongoc_cursor_error [%s]", error.message);
        rtn = -1;
        goto cleanup;
    }

    if (!mongoc_collection_find_and_modify(collection, query, NULL, update, NULL, false, false, true, &reply, &error)) {
        err("mongoc_collection_find_and_modify error [%s]", error.message);
        rtn = -1;
        goto cleanup;
    }

    rtn = 0;

cleanup:
    bson_destroy(query);
    bson_destroy(update);
    bson_destroy(&reply);

    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return NULL;
}
