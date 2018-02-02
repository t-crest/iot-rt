/*
  Test of SLIP
*/
#include "config.h"

void tpip_slip(char *);

int main(int argc, char *argv[]) {

  if (argc != 2) return -1;
  tpip_slip(argv[1]);
  return 0;

  // this shall be called periodically
  LL_RUN();

  // just call all to check if everything compiles
  unsigned char buf[100];

  int valid = LL_RXFULL();
  int len = LL_RXREAD(buf);
  valid = LL_TXEMPTY();
  LL_TXWRITE(buf, 10);
}
