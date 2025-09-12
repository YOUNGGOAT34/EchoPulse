#include "hexadump.h"

#include "send_raw.h"

/*
  Add host resolution
  Add --ping--statistics--
  add the ping thing to show that how man bytes we are pinging to a certain ip???????
  What if it can ping multiple hosts ,,and possibly ping the entire subnet???
  
*/


volatile sig_atomic_t keep_sending=0;

void handle_sigInt(__attribute__((unused)) i32 sig){
    
   keep_sending=1;
}


int main(int argc,char *argv[]){

     if(argc<2){
          fprintf(stderr,RED"Usage: ./main <destination ip address>\n"RESET);
          exit(EXIT_SUCCESS);
     }
     
     signal(SIGINT,handle_sigInt);

     u8 *data = (u8 *)"Hello";
     u16 data_size = strlen((char *)data);
     
     icmp *packet = create_icmp_packet(echo,data, data_size);
     // print_icmp_packet(packet, sizeof(icmp) + data_size);
     u8 *raw=create_raw_icmp(packet);
     // u16 size=sizeof(raw_icmp)+data_size;
     if(!raw){
          printf("Empty\n");
          exit(1);
     }
   
     // for(int i=1;i<argc;i++){
          
              
     // }

     char *ip=argv[1];
          
     IP *pkt=create_ip_packet(ICMP,3000,ip);

     pkt->payload=packet;

     u8 *raw_bytes=create_raw_ip(pkt);
     // u16 _size=sizeof(RAWIP)+sizeof(raw_icmp)+pkt->payload->size;
     // while(!keep_sending){
     //      send_raw_ip(pkt);
     //      sleep(1);
     // }
  
      
     STATS *stats=send_packets(pkt,&keep_sending);
     
     printf("\n");
     printf(YELLOW"---%s statistics---\n %lld packets transmitted, %lld received, %.1f%% packet loss,time %lldms\n",
           print_ip(inet_addr(ip)),
           stats->packets_sent,
           stats->packets_received,
           100.0*(stats->packets_sent-stats->packets_received)/stats->packets_sent,
           stats->duration_ms
     );

     printf("rtt min/avg/max/mdev=%lld/%lld/%lld/%lld ms\n"RESET,stats->min_rtt,stats->avg_rtt,stats->max_rtt,stats->mdev_rtt);
     printf("\n");
     //memory freeing
     free(pkt);
     free(raw_bytes);
     free(packet);
     free(raw);
     return 0;

}