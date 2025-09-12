#ifndef SEND_RAW_H
#define SEND_RAW_H

#include "ip.h"

typedef struct{
    i64 packets_sent;
    i64 packets_received;

    i64 duration_ms;
    i64 min_rtt;
    i64 max_rtt;
    i64 total_rtt;
    i64 avg_rtt;
    i64 mdev_rtt;

}STATS; 

typedef struct {
   i64 *rtts;
   i64 count;
   i64 capacity;
}RTTsBuffer;

void send_raw_ip(IP *,STATS *,RTTsBuffer *);
ssize_t recv_ip_packet(i32);
STATS *send_packets(IP *pkt,volatile sig_atomic_t *);

#endif