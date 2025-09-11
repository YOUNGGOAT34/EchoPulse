#include "hexadump.h"

#include "send_raw.h"

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
   

     char *ip=argv[1];

     IP *pkt=create_ip_packet(ICMP,3000,ip);

     pkt->payload=packet;

     u8 *raw_bytes=create_raw_ip(pkt);
     // u16 _size=sizeof(RAWIP)+sizeof(raw_icmp)+pkt->payload->size;
     while(!keep_sending){

          send_raw_ip(pkt);
     }
     //memory freeing
     free(packet);
     free(pkt);
     free(raw);
     free(raw_bytes);

     return 0;
}