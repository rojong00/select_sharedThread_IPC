#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#define MAXBUF 256
#define NAME_LEN 100

typedef struct stSharedMtx
{
    pthread_mutex_t *ptr;
    int shm_fd;
    char* name;
    int chkCreated;
} sharedMtx;

sharedMtx shared_mutex_init(char *name);
int shared_mutex_lock(const sharedMtx* mtx);
int shared_mutex_unlock(const sharedMtx* mtx);
int shared_mutex_destroy(sharedMtx* mtx);
int shared_mutex_close(sharedMtx* mtx);

#endif
