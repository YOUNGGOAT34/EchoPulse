#include "hexadump.h"
#include "ip.h"


int main(){
     u8 *data = (u8 *)"Hello";
     u16 data_size = strlen((char *)data);
     
     icmp *packet = create_icmp_packet(echo,data, data_size);
     // print_icmp_packet(packet, sizeof(icmp) + data_size);

     u8 *raw=create_raw_icmp(packet);
     u16 size=sizeof(raw_icmp)+data_size;
     if(!raw){
          printf("Empty\n");
     }
     hexadump((void *)raw,size);

    
     IP *pkt=create_ip_packet(ICMP,3000,"192.3.4.0");
     pkt->payload=packet;

     u8 *raw_bytes=create_raw_ip(pkt);
     u16 _size=sizeof(RAWIP)+sizeof(raw_icmp)+pkt->payload->size;
     print_ip_packet(pkt);

     hexadump(raw_bytes,_size);

     
     


     return 0;
}