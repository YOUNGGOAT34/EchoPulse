#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdlib.h>
#include "queue.h"

typedef struct{
   pthread_t *threads;
   i32 thread_count;
   QUEUE q;
   pthread_mutex_t lock;
   pthread_cond_t notify;
   bool shutdown;
}POOL;


void *worker_thread(void *arg);
POOL *threadpool_create(i32 thread_count, i32 queue_capacity);
i32 threadpool_add(POOL *pool, void (*function)(void *), void *arg);
i32 threadpool_add(POOL *pool, void (*function)(void *), void *arg);
void threadpool_destroy(POOL *pool);  

#endif