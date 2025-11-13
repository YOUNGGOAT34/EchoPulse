#include "queue.h"

void initialize_queue(QUEUE *q,i32 capacity){
      q->tasks=malloc(sizeof(QUEUE)*capacity);
      q->capacity=capacity;
      q->front=0;
      q->back=0;
      q->tasks_count=0;
}