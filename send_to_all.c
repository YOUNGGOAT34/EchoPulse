#include "hexadump.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include<stdlib.h>
#include "ip.h"
#include "send_to_all.h"




i32 get_iface_ip_mask(in_addr_t *mask,in_addr_t *current_ip) {
  
   i32 fd = socket(AF_INET, SOCK_DGRAM, 0);
   if (fd < 0) return -1;
   
   struct ifreq if_request;
   strncpy(if_request.ifr_name,"wlan0", IFNAMSIZ);

   // Get IP
   if (ioctl(fd, SIOCGIFADDR, &if_request) < 0) return -1;
   *current_ip = ((struct sockaddr_in *)&if_request.ifr_addr)->sin_addr.s_addr;
    
   // Get Netmask
   if (ioctl(fd, SIOCGIFNETMASK, &if_request) < 0) return -1;
   *mask = ((struct sockaddr_in *)&if_request.ifr_netmask)->sin_addr.s_addr;

   close(fd);
   return 0;
}


range *compute_subnet_range(in_addr_t ip, in_addr_t mask) {
   in_addr_t start_ip_address = ip & mask;
   in_addr_t end_ip_address = start_ip_address | ~mask;
   start_ip_address = htonl(ntohl(start_ip_address) + 1);
   end_ip_address = htonl(ntohl(end_ip_address) - 1);

   range *r=malloc(sizeof(range));
   r->start=start_ip_address;
   r->end=end_ip_address;
   return r;
}


/*
   producer-consumer
   producer will produce packets ,and consumer will take them and send them
*/

pthread_mutex_t QueueMutex;
pthread_cond_t QueueFullCond;
pthread_cond_t QueueEmptyCond;

queue *q;

void *produce_packets(void *arg){

      in_addr_t current_ip;
       in_addr_t netmask;
   
     
      get_iface_ip_mask(&netmask,&current_ip);
      range *_range=compute_subnet_range(current_ip,netmask);
      IP *packet=(IP *)arg;


    while(1){
       
       
       for(in_addr_t i=_range->start;i<_range->end+1;i++){
             
              pthread_mutex_lock(&QueueMutex);
              while(!can_push(q)){

                 pthread_cond_wait(&QueueEmptyCond,&QueueMutex);
              }

              IP *copy=malloc(sizeof(IP));
              *copy=*packet;
              copy->dst=i;

             
              push(copy,q);
              pthread_mutex_unlock(&QueueMutex);
              pthread_cond_signal(&QueueFullCond);
              
       }
    }
    

     free(packet);
     free(_range);
    
}

void *consume_packets(void *arg){

      i32 sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
      int one = 1;
      setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
      while(1){

        
   
       
           pthread_mutex_lock(&QueueMutex);
         
         while(empty(q)){
            pthread_cond_wait(&QueueFullCond,&QueueMutex);
         }
        
       
         IP *packet=pop(q);
         pthread_mutex_unlock(&QueueMutex);
         pthread_cond_signal(&QueueEmptyCond);
         
   
         if(!packet){
             fprintf(stderr,"cannot send a null packet\n");
            //  return;
         }
   
         u8 *raw_ip=create_raw_ip(packet,NULL);
         if(!raw_ip){
             fprintf(stderr,"Failed to create raw IP bytes\n");
            //  return;
         }
   
   
         struct sockaddr_in dst;
        dst.sin_family=AF_INET;
        dst.sin_addr.s_addr=packet->dst;   
         
        size_t size=sizeof(RAWIP)+sizeof(raw_icmp);
        if(packet->payload){
            size+=packet->payload->size;
        }
   
        
        ssize_t bytes_sent=sendto(sockfd,raw_ip,size,0,(const struct sockaddr *)&dst,sizeof(dst));
          free(raw_ip);
          free(packet);
        if(bytes_sent<0){
            error("sending raw ip packet\n");
        }


      }


      /*
         You probably want to:

ignore EHOSTUNREACH

ignore ENETUNREACH

continue sending
      
      */
    
      close(sockfd);
     


}

void start_threads(IP *packet){
    
 
  q=malloc(sizeof(queue));
  q->size=0;

    pthread_t threads[NUM_OF_THREADS];

      pthread_mutex_init(&QueueMutex,NULL);
      pthread_cond_init(&QueueFullCond,NULL);
      pthread_cond_init(&QueueEmptyCond,NULL);
    
       for(i32 i=0;i<NUM_OF_THREADS;i++){
           if (i==0){
               pthread_create(&threads[i],NULL,&produce_packets,packet);
           }else{
              pthread_create(&threads[i],NULL,&consume_packets,NULL);
           }
       }

      // pthread_cond_destroy(&QueueEmptyCond);
      // pthread_cond_destroy(&QueueFullCond);
      // pthread_mutex_destroy(&QueueMutex);

}



//implementation of queue functions
void push(IP *packet,queue *q){
   if(q->size>=MAX_SIZE){
       fprintf(stderr,"The queue is full\n");
       return;
   }
   q->packets[q->size++]=packet;

}

IP *pop(queue *q){
    if(empty(q)){
       fprintf(stderr,"popping from an empty queue\n");
       return NULL;
    }
    IP *packet=q->packets[0];
    if(!packet){
       fprintf(stderr,"Empty packet in the queue\n");
       return NULL;
    }

    
    for(i32 i=0;i<q->size-1;i++){
         q->packets[i]=q->packets[i+1];
    }

    q->size--;

    return packet;
}
bool empty(queue *q){
     return  q->size==0;
}

bool can_push(queue *q){
    return q->size<MAX_SIZE;
}
