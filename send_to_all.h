#ifndef SEND_TO_ALL_H
#define SENT_TO_ALL_H



i32 get_iface_ip_mask(in_addr_t *mask,in_addr_t *current_ip);
void compute_subnet_range(in_addr_t ip, in_addr_t mask); 

#endif