#ifndef QUEUE_H
#define QUEUE_H
#include "icmp.h"

typedef struct{
   void (*function)(void *);

}TASK;

typedef struct{
     TASK *tasks;
     i32 tasks_count;
     i32 capacity;
     i32 front;
     i32 back;
}QUEUE;


void initialize_queue(QUEUE *q,i32 capacity);

#endif