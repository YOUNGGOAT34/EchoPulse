#ifndef MAIN_H
#define MAIN_H

#include "hexadump.h"
#include "send_raw.h"
#include <getopt.h>

typedef struct{
     i64 count;
     bool quiet;
}options;

int main(i32,i8 *[]);
void command_parser(i8 ,i8 *[]);
void handle_sigInt(__attribute__((unused)) i32 sig);
void help(void);
void double_hyphen(i32,i8 *[]);

#endif