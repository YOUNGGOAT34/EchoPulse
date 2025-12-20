#ifndef SEND_TO_ALL_H
#define SENT_TO_ALL_H

#define MAX_SIZE 200

typedef struct{

   i32 size;
   IP *packets[MAX_SIZE];

}queue;

// queue functions
void push(IP *packet,queue *q);
IP *pop(queue *q);
bool empty(queue *q);


i32 get_iface_ip_mask(in_addr_t *mask,in_addr_t *current_ip);
void compute_subnet_range(in_addr_t ip, in_addr_t mask); 

#endif