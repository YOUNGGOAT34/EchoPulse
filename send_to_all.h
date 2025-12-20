#ifndef SEND_TO_ALL_H
#define SENT_TO_ALL_H

#define MAX_SIZE 200
#define NUM_OF_THREADS 10

typedef struct{

   i32 size;
   IP *packets[MAX_SIZE];

}queue;

typedef struct{
   in_addr_t start;
   in_addr_t end;
}range;

// queue functions
void push(IP *packet,queue *q);
IP *pop(queue *q);
bool empty(queue *q);




void start_threads(IP *packet);

i32 get_iface_ip_mask(in_addr_t *mask,in_addr_t *current_ip);
range *compute_subnet_range(in_addr_t ip, in_addr_t mask); 
bool can_push(queue *q);

#endif