#include "hexadump.h"


int main(){
     uint8 *data = (uint8 *)"goatping";
     uint16 data_size = strlen((char *)data);
     icmp *packet = create_icmp_packet(8, 0, data, data_size);
     packet->checksum = checksum(packet, sizeof(icmp) + data_size);
     print_icmp_packet(packet, sizeof(icmp) + data_size);
     return 0;
}