/*
  Test of SLIP
*/

#include <stdio.h>

#include "config.h"

void tpip_slip(char *);

void tpip_print(unsigned char buf[], int len) {
  
  printf("One packet:\n");
  for (int i=0; i<len; ++i) {
    printf("%02x ", buf[i]);
    if (i%4 == 3) printf("\n");
  }
  printf("\n\n");
}

int main(int argc, char *argv[]) {

  // TODO: there should be constants in config.h
  unsigned char buf[2000];

  if (argc != 2) return -1;
  LL_INIT(argv[1]);

  // this shall be executed periodically
  for (;;) {
    LL_RUN();
    if (LL_RXFULL()) {
      int len = LL_RXREAD(buf);
      tpip_print(buf, len);
    }
  }

  // just call all to check if everything compiles

  int valid = LL_RXFULL();
  int len = LL_RXREAD(buf);
  valid = LL_TXEMPTY();
  LL_TXWRITE(buf, 10);
}
