#ifndef MAIN_H
#define MAIN_H

#include "hexadump.h"
#include "send_raw.h"
#include <getopt.h>


int main(i32,i8 *[]);
void command_parser(i8 ,i8 *[]);
void handle_sigInt(__attribute__((unused)) i32 sig);
void help(void);

#endif