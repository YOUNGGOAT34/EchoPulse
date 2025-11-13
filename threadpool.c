
#include "threadpool.h"


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
      pop(&pool->q,&task);
       pthread_mutex_unlock(&pool->lock);

       ((*(task.function)))(task.arg);

    }

    return NULL;
}


POOL *threadpool_create(i32 thread_count, i32 queue_capacity) {
    POOL *pool = malloc(sizeof(POOL));
    if (!pool) return NULL;

    pool->thread_count = thread_count;
    pool->shutdown = false;
    
    initialize_queue(&pool->q, queue_capacity);
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->notify, NULL);

    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&pool->threads[i], NULL,worker_thread, pool);
    }

    return pool;
}



i32 threadpool_add(POOL *pool, void (*function)(void *), void *arg) {
    pthread_mutex_lock(&pool->lock);

    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->lock);
        return -1;
    }

 
    TASK task = { function, arg };
    if (enqueue(&pool->q, task) == -1) {
        pthread_mutex_unlock(&pool->lock);
        return -1; 
    }

    pthread_cond_signal(&pool->notify);
    pthread_mutex_unlock(&pool->lock);
    return 0;
}



void threadpool_destroy(POOL *pool) {
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = true;

    
    pthread_cond_broadcast(&pool->notify);
    pthread_mutex_unlock(&pool->lock);

 
    for (int i = 0; i < pool->thread_count; i++)
        pthread_join(pool->threads[i], NULL);

 
    free(pool->threads);
    destroy_queue(&pool->q);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
    free(pool);
}