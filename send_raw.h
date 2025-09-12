#ifndef SEND_RAW_H
#define SEND_RAW_H

#include "ip.h"

typedef struct STATS{
    i64 packets_sent;
    i64 packets_received;
    i64 duration_ms;
}STATS;

void send_raw_ip(IP *,i64 *,i64 *,i64 *);
ssize_t recv_ip_packet(i32);
STATS *send_packets(IP *pkt,volatile sig_atomic_t *);

#endif