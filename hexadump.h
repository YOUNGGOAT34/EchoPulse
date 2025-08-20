#ifndef HEXADUMP_H
#define HEXADUMP_H
#include "ping.h"


#define IS_PRINTABLE_ASCII(c) (((c)>31) && ((c)<127))

void hexadump(void *buff,uint16 size);
#endif