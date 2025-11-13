#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdlib.h>
#include "queue.h"

typedef struct{
   pthread_t *threads;
   i32 thread_count;
   QUEUE q;
   pthread_mutex_lock lock;
   pthread_cond_t notify;
   bool shutdown;
}POOL;



#endif