#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>

#include "send_raw.h"

void add_rtt(RTTsBuffer *rtts,long rtt){
    if(rtts->count==rtts->capacity){
        rtts->capacity=(rtts->capacity==0)?16:rtts->capacity*2;
        rtts->rtts=realloc(rtts->rtts,rtts->capacity *sizeof(long));

        if(!rtts->rtts){
           fprintf(stderr,"Failed to allocate memory for rtts buffer: \n");
           return;
        }
    }

    rtts->rtts[rtts->count++]=rtt;
}


STATS *send_n_packets(IP *packet,options *opts,volatile sig_atomic_t *sig){
      STATS *stats=malloc(sizeof(STATS));
      if(!stats){
          fprintf(stderr,RED"Failed to allocate memory fo stats: %s\n"RESET,strerror(errno));
          return NULL;
     }

     stats->packets_received=0;
     stats->packets_sent=0;
     stats->duration_ms=0;
     stats->min_rtt=INT_MAX;
     stats->max_rtt=INT_MIN;
     stats->total_rtt=0;
     stats->mdev_rtt=0;

     RTTsBuffer *rttbuffer=malloc(sizeof(RTTsBuffer));
     if(!rttbuffer){
          fprintf(stderr,"Failed to allocate memory for rttbuffer: %s\n",strerror(errno));
          return NULL;
     }
     rttbuffer->capacity=0;
     rttbuffer->count=0;

     int i=0;
     struct timeval start,end;
     gettimeofday(&start,NULL);
     while(i<opts->count && !(*sig)){
          send_raw_ip(packet,stats,rttbuffer,opts);
          i++;
          sleep(1);
     }

     gettimeofday(&end,NULL);
     stats->duration_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

      return stats;
}



STATS *send_packets(IP *pkt,volatile sig_atomic_t *sig,options *opts){
     STATS *stats=malloc(sizeof(STATS));
     if(!stats){
          fprintf(stderr,RED"Failed to allocate memory for stats: %s\n"RESET,strerror(errno));
          return NULL;
     }

     stats->packets_received=0;
     stats->packets_sent=0;
     stats->duration_ms=0;
     stats->min_rtt=INT_MAX;
     stats->max_rtt=INT_MIN;
     stats->total_rtt=0;
     stats->mdev_rtt=0;
      
     RTTsBuffer *rttbuffer=malloc(sizeof(RTTsBuffer));
     if(!rttbuffer){
          fprintf(stderr,"Failed to allocate memory for rttbuffer: %s\n",strerror(errno));
          return NULL;
     }
     rttbuffer->capacity=0;
     rttbuffer->count=0;
     
     struct timeval start,now,end;
     u64 sleep_time=(opts->interval>0)?opts->interval:1;
     gettimeofday(&start,NULL);

     while(!(*sig)){
          /*
             So if the user passed a -w or --time option ,the program should exit when it times out
          */
          if(opts->time>0){
               gettimeofday(&now,NULL);

               double time_taken=(now.tv_sec-start.tv_sec)+(now.tv_usec-start.tv_usec)/1000000;

               if(time_taken>=opts->time){
                    break;
               }
          }
          send_raw_ip(pkt,stats,rttbuffer,opts);
          sleep(sleep_time);
     }

     gettimeofday(&end,NULL);

     stats->duration_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
     
     stats->avg_rtt=(stats->packets_received > 0)?stats->total_rtt/stats->packets_received:0;

     double sum_sq_dev=0;
     for(i64 i=0;i<rttbuffer->count;i++){
          double diff=(double)rttbuffer->rtts[i]-(double)stats->avg_rtt;
          sum_sq_dev+=diff*diff;
     }
    
     // stats->mdev_rtt=(stats->packets_received>0)? sum_sq_dev/stats->packets_received :0;
     stats->mdev_rtt=(rttbuffer->count>1)?sqrt(sum_sq_dev/rttbuffer->count):0;
     free(rttbuffer->rtts);
     free(rttbuffer);
     return stats;
}



void send_raw_ip(IP *packet,STATS *stats,RTTsBuffer *rtts,options *opts){
     struct timeval start,end;
     if(!packet){
       error("Cannot send a null packet\n");
     }
     i32 sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
     if(sockfd<0){
        error("Raw socket creation failed\n");
     }

     struct timeval tv;
     tv.tv_sec = opts->timeout;
     setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
     
     i32 one=1;
     if(setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&one,sizeof(one))<0){
        error("Failed to set socket options\n");
     }

     u8 *raw_ip=create_raw_ip(packet,opts);
     if(!raw_ip){
        error("Failed to create raw ip bytes\n");
     }
      
     struct sockaddr_in dst;
     dst.sin_family=AF_INET;
     dst.sin_addr.s_addr=packet->dst;   
      
     size_t size=sizeof(RAWIP)+sizeof(raw_icmp);
     if(packet->payload){
         size+=packet->payload->size;
     }

     gettimeofday(&start,NULL);
     ssize_t bytes_sent=sendto(sockfd,raw_ip,size,0,(const struct sockaddr *)&dst,sizeof(dst));
     if(bytes_sent<0){
         error("sending raw ip packet\n");
     }


     ssize_t received_bytes=recv_ip_packet(sockfd,opts,packet->dst);
     gettimeofday(&end,NULL);
     double rtt=((end.tv_sec-start.tv_sec)*1000.0L)+((end.tv_usec-start.tv_usec)/1000.0L);
     stats->packets_sent++;

     if(received_bytes>=0){
          add_rtt(rtts,rtt);
          stats->min_rtt=(rtt<stats->min_rtt)?rtt:stats->min_rtt;
          stats->max_rtt=(rtt>stats->max_rtt)?rtt:stats->max_rtt;
          
          stats->total_rtt+=rtt;
          
          stats->packets_received++;
          if(!opts->quiet){
               printf("time=%.1f ms "RESET,rtt);
               printf("\n");
          }
     }

     free(raw_ip);
     close(sockfd);
  
}


ssize_t recv_ip_packet(i32 sockfd,options *opts,u32 dst_ip){
  
   i8 buffer[65536];
   struct sockaddr_in src_addr;
   socklen_t addr_len=sizeof(src_addr);
   size_t buff_len=sizeof(buffer);
   ssize_t bytes_received=recvfrom(sockfd,buffer,buff_len,0,(struct sockaddr *)&src_addr,&addr_len);
   
   if(bytes_received<0){
      if(errno==EAGAIN || errno==EWOULDBLOCK){
     //     printf(RED"Request timed out %s\n"RESET,strerror(errno));
         return bytes_received;
      }else if(errno==ECONNREFUSED){
         printf(RED"Connection refused\n"RESET);
      }else if(errno==EINTR){
           return bytes_received;
      }else{
         error("Response Error\n");
      }
    
   }

   RAWIP *res=(RAWIP *)buffer;

   raw_icmp *ricmp=(raw_icmp *)(buffer+(res->ihl*4));
   i8 *src_ip=print_ip(src_addr.sin_addr.s_addr);
   i8 *dst__ip=print_ip(dst_ip);

   if(ricmp->type==0 && ricmp->code==0 && strcmp(dst__ip,src_ip)==0){
            if(!opts->quiet){


                 printf(GREEN "\n%ld bytes ",bytes_received-(res->ihl*4));
                 printf("from %s: ",src_ip);
                 printf("icmp_seq=%hd ",ntohs(ricmp->sequence));
                 
                 free(dst__ip);
                 free(src_ip);
            }
            
   }else if(ricmp->type==3){
      switch(ricmp->code){
         //some cases will come up later as I advance the project,right now I'm just including all of them
             case 0:
             printf(RED"Destination network unreachable\n"RESET);
                  exit(1);
             case 1:
             printf(RED"Destination host unreachable\n"RESET);
                  exit(1);
             case 2:
             printf(RED"Destination protocal unreachable\n"RESET);
                  exit(1);
             case 3:
             printf(RED"Destination port unreachable\n"RESET);
                  exit(1);
             case 4:
             printf(RED"Fragmentation required, and DF flag set \n"RESET);
                  exit(1);
             case 5:
             printf(RED"Source route failed \n"RESET);
                  exit(1);
             case 6:
             printf(RED"Destination network unknown \n"RESET);
                  exit(1);
             case 7:
             printf(RED"Destination host unknown \n"RESET);
                  exit(1);
             case 8:
             printf(RED"Source host isolated \n"RESET);
                  break;
             case 9:
             printf(RED"Network administratively prohibited \n"RESET);
                  exit(1);
             case 10:
             printf(RED"Host administratively prohibited \n"RESET);
                  break;
             case 11:
             printf(RED"Network unreachable for ToS\n"RESET);
                  exit(1);
             case 12:
             printf(RED"Host unreachable for ToS\n"RESET);
                  exit(1);
             case 13:
             printf(RED"Communication administratively prohibited\n"RESET);
                  exit(1);
             case 14:
             printf(RED"Host Precedence Violation \n"RESET);
                  exit(1);
             case 15:
             printf(RED"Precedence cutoff in effect\n"RESET);
                  exit(1);

            
      }
      
   }else{
      free(dst__ip);
      free(src_ip);
      return -1;
   }

  return bytes_received;

}