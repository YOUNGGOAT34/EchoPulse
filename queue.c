#include "queue.h"

void initialize_queue(QUEUE *q,i32 capacity){

      q->tasks=malloc(sizeof(QUEUE)*capacity);
      q->capacity=capacity;
      q->front=0;
      q->back=0;
      q->tasks_count=0;

}

i32 enqueue(QUEUE *q,TASK task){
    if(q->capacity==q->tasks_count){
       return -1;
    }

    q->tasks[q->back]=task;
    q->back=(q->back+1)% q->capacity;
    q->tasks_count++;
    return 0;

}