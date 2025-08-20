#include "hexadump.h"


int main(){
     uint16 data[] = {0x0001, 0xF203, 0xF4F5, 0xF6F7};
  
     printf("%04hX\n",_checksum(data,4));
}