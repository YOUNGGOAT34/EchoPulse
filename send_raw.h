#ifndef SEND_RAW_H
#define SEND_RAW_H

#include "ip.h"

typedef struct{
    i64 packets_sent;
    i64 packets_received;

    double duration_ms;
    double min_rtt;
    double max_rtt;
    double total_rtt;
    double avg_rtt;
    double mdev_rtt;

}STATS; 

typedef struct {
   i64 *rtts;
   i64 count;
   i64 capacity;
}RTTsBuffer;

void send_raw_ip(IP *,STATS *,RTTsBuffer *);
ssize_t recv_ip_packet(i32);
STATS *send_packets(IP *pkt,volatile sig_atomic_t *);
STATS *send_n_packets(IP *,i32);

#endif