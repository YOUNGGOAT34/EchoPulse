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
#include <errno.h>
#include "ip.h"
#include "send_to_all.h"
#include "main.h"




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
   start_ip_address = (ntohl(start_ip_address) + 1);
   end_ip_address =(ntohl(end_ip_address) - 1);

   range *r=malloc(sizeof(range));
   r->start=(start_ip_address);
   r->end=(end_ip_address);
   return r;
}




/*
   producer-consumer
   producer will produce packets ,and consumer will take them and send them
*/

pthread_mutex_t QueueMutex;
pthread_mutex_t printMutex;
pthread_cond_t QueueFullCond;
pthread_cond_t QueueEmptyCond;
extern volatile sig_atomic_t keep_sending;

volatile sig_atomic_t producer_done=0;

queue *q;

void *produce_packets(void *arg){
   

      in_addr_t current_ip;
       in_addr_t netmask;
   
     
      get_iface_ip_mask(&netmask,&current_ip);
      range *_range=compute_subnet_range(current_ip,netmask);
      IP *packet=(IP *)arg;

      printf("Start Ip: %s\n",print_ip(_range->start));
      printf("Start Ip: %s\n",print_ip(_range->end));

    while(!keep_sending){
       
       
       for(in_addr_t i=_range->start;i<_range->end+1;i++){
             
              pthread_mutex_lock(&QueueMutex);
              while(!can_push(q) && !keep_sending){

                 pthread_cond_wait(&QueueEmptyCond,&QueueMutex);
              }

              if(keep_sending){
                  pthread_mutex_unlock(&QueueMutex);
                  break;
              }

              IP *copy=malloc(sizeof(IP));
              *copy=*packet;
              copy->dst=htonl(i);
              push(copy,q);
              pthread_mutex_unlock(&QueueMutex);
              pthread_cond_signal(&QueueFullCond);

       }

       producer_done=1;
       

       pthread_cond_broadcast(&QueueFullCond);
       printf("Finished scanning\n");

       break;

   
       
    }



     free(packet);
     free(_range);

     return NULL;
    
}

void *consume_packets(void *arg){
       (void)arg;

      i32 sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
      if(sockfd<1){
           error("Failed to create socket");
      }
      int one = 1;
      setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
      while(!keep_sending){
            
           pthread_mutex_lock(&QueueMutex);
         
         while(empty(q) && !producer_done){
            pthread_cond_wait(&QueueFullCond,&QueueMutex);
         }


         if(empty(q) && producer_done){
             pthread_mutex_unlock(&QueueMutex);
             break;
         }
  
         if(keep_sending){
            pthread_mutex_unlock(&QueueMutex);

            break;
         }
        
       
         IP *packet=pop(q);
         pthread_mutex_unlock(&QueueMutex);
         pthread_cond_signal(&QueueEmptyCond);
         
   
         if(!packet){
             fprintf(stderr,"cannot send a null packet\n");
             continue;
         }
   
         u8 *raw_ip=create_raw_ip(packet,NULL);
         if(!raw_ip){
             fprintf(stderr,"Failed to create raw IP bytes\n");
             continue;
         }



         struct sockaddr_in dst;
         dst.sin_family=AF_INET;
         dst.sin_addr.s_addr=packet->dst;   
         


        size_t size=sizeof(RAWIP)+sizeof(raw_icmp);
        if(packet->payload){
            size+=packet->payload->size;
        }
   
        
        ssize_t bytes_sent=sendto(sockfd,raw_ip,size,0,(const struct sockaddr *)&dst,sizeof(dst));
        pthread_mutex_lock(&printMutex);
         pthread_mutex_unlock(&printMutex);
         //  free(raw_ip);
         //  free(packet);
        if(bytes_sent<1){
            
            if(errno==EHOSTUNREACH || errno== ENETUNREACH){
               continue;
               
            }
        }



            i8 buffer[65536];
            struct sockaddr_in src_addr;
            socklen_t addr_len=sizeof(src_addr);
            size_t buff_len=sizeof(buffer);
            struct timeval tv = {0, 500000};

            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
            ssize_t bytes_received=recvfrom(sockfd,buffer,buff_len,0,(struct sockaddr *)&src_addr,&addr_len);

            if(bytes_received<0){
                            if(errno==EAGAIN || errno==EWOULDBLOCK){
                                 
                              }else if(errno==ECONNREFUSED){
                                 printf(RED"Connection refused\n"RESET);
                              }else if(errno==EINTR){
                                 // return bytes_received;
                              }else{
                               printf("Response Error %s ,%d\n",strerror(errno),sockfd);
                            }

                            continue;
            }

             RAWIP *res=(RAWIP *)buffer;

            raw_icmp *response=(raw_icmp*)(buffer + (res->ihl*4));

            if(response->type==0 && packet->dst==res->src){
                       printf("Found host %s\n",print_ip(packet->dst));
            }

            

      }


    
      close(sockfd);
     
        return NULL;

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

       for(i32 i=0;i<NUM_OF_THREADS;i++){
         
               pthread_join(threads[i],NULL);
            
       }

       while(!empty(q)){
          IP *p = pop(q);
          free(p);
      }
      free(q);
  
      pthread_cond_destroy(&QueueEmptyCond);
      pthread_cond_destroy(&QueueFullCond);
      pthread_mutex_destroy(&QueueMutex);


      // in_addr_t current_ip;
      // in_addr_t netmask;
   
     
      // get_iface_ip_mask(&netmask,&current_ip);
      // range *_range=compute_subnet_range(current_ip,netmask);
      // i32 counter=0;
      // for(in_addr_t i=(_range->start);i<(_range->end);i++){
      //      counter++;
      //      printf("%s\n",print_ip(htonl(i)));
      // }

      // printf("%d\n",counter);


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
