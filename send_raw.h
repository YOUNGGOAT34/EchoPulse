#ifndef SEND_RAW_H
#define SEND_RAW_H

#include "ip.h"
#include <math.h>

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

void send_raw_ip(IP *,STATS *,RTTsBuffer *,options *);
ssize_t recv_ip_packet(i32,options *,u32 );
STATS *send_packets(IP *pkt,volatile sig_atomic_t *,options *opts);
STATS *send_n_packets(IP *,options *,volatile sig_atomic_t *);

#endif