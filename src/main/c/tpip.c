/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU
  Authors: Martin Schoeberl
  License: Simplified BSD
*/

#include <stdio.h>

#define MAX_BUF_NUM 4
#define MAX_BUF_SIZE 1024

// Fixed, static buffers to enable WCET analysis
static int buffer[MAX_BUF_NUM][MAX_BUF_SIZE];

int main() {
  printf("Hello tpip!\n");

  return 0;
}
