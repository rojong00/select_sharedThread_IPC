#include "wrapSharedMutex.h"

int shared_mutex_lock(const sharedMtx* mtx)
{
    int ret = pthread_mutex_lock(mtx->ptr);
    if (ret == EOWNERDEAD)
    {
        ret = pthread_mutex_consistent(mtx->ptr);
        if (ret != 0)
        {
            perror("pthread_mutex_consistent");
        }
    }

    return ret;
}

int shared_mutex_unlock(const sharedMtx* mtx)
{
    return pthread_mutex_unlock(mtx->ptr);
}

sharedMtx shared_mutex_init(char *name)
{
    sharedMtx mtx = {NULL, 0, NULL, 0};

    //if ((mtx.shm_fd = shm_open(name, O_RDWR, 0660)) < 0)
    if ((mtx.shm_fd = shm_open(name, O_RDWR, 0660)) < 0)
    {
        mtx.shm_fd = shm_open(name, O_RDWR | O_CREAT, 0660);
        mtx.chkCreated = 1;
    }
    else
    {
        mtx.chkCreated = 1;
    }

    if (ftruncate(mtx.shm_fd, sizeof(pthread_mutex_t)) != 0)
    {
        perror("shared_mutex_init : ftruncate");
        return mtx;
    }

    void *addr = mmap(
            NULL, sizeof(pthread_mutex_t),
            PROT_READ | PROT_WRITE, MAP_SHARED, mtx.shm_fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("shared_mutex_init : mmap");
        return mtx;
    }
    pthread_mutex_t *mtx_ptr = (pthread_mutex_t*)addr;


	if (mtx.chkCreated)
    {
	    pthread_mutexattr_t attr;
	    if (pthread_mutexattr_init(&attr))
        {
	        perror("shared_mutex_init : pthread_mutexattr_init");
	        return mtx;
	    }
	    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED))
        {
	        perror("shared_mutex_init : pthread_mutexattr_setpshared");
	        return mtx;
	    }
		if (pthread_mutex_init(mtx_ptr, &attr))
        {
		    perror("shared_mutex_init : pthread_mutex_init");
		    return mtx;
		}
	}
    mtx.ptr = mtx_ptr;
	mtx.name = (char *)malloc(NAME_LEN+1);
	strcpy(mtx.name, name);
	return mtx;
}
    

int shared_mutex_close(sharedMtx* mtx)
{
	if (munmap((void *)mtx->ptr, sizeof(pthread_mutex_t))) {
	    perror("munmap");
	    return -1;
	}
	mtx->ptr = NULL;

	if (close(mtx->shm_fd))
    {
	    perror("close");
	    return -1;
	}
	mtx->shm_fd = 0;

	free(mtx->name);

	return 0;
}

int shared_mutex_destroy(sharedMtx* mtx)
{
    if ((errno = pthread_mutex_destroy(mtx->ptr)))
    {
        perror("pthread_mutex_destroy");
        return -1;
    }

    if (munmap((void*)mtx->ptr, sizeof(pthread_mutex_t)))
    {
        perror("pthread_mutex_destroy : munmap");
        return -1;
    }
    mtx->ptr = NULL;

    if (close(mtx->shm_fd))
    {
        perror("pthread_mutex_destroy : close");
        return -1;
    }
    mtx->shm_fd = 0;

    if (shm_unlink(mtx->name))
    {
        perror("pthread_mutex_destroy : unlink");
        return -1;
    }
    free(mtx->name);

    return 0;
}
