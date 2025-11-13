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


void *worker_thread(void *arg){
    POOL *pool=(POOL *)arg;

    while(1){
      pthread_mutex_lock(&pool->lock);
      while(pool->q.tasks_count==0 && !pool->shutdown){
          pthread_cond_wait(&pool->notify,&pool->lock);
      }

      if(pool->shutdown){
         pthread_mutex_unlock(&pool->lock);
         pthread_exit(NULL);
      }

      TASK task;
      pop(pool->q,&task);
       pthread_mutex_unlock(&pool->lock);

       ((*(task.function)))(task.arg);

    }

    return NULL;
}

#endif