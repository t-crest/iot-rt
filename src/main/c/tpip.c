/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl, Rasmus Ulslev Pedersen
  License: Simplified BSD
*/

#include <stdio.h>

#define MAX_BUF_NUM 4
#define MAX_BUF_SIZE 1024 // bytes in multiples of 4

// Fixed, static buffers to enable WCET analysis
static int buffer[MAX_BUF_NUM][MAX_BUF_SIZE/4];

int main() {
  printf("Hello tpip world!\n");

  return 0;
}
