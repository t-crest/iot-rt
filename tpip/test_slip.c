/*
  Test of SLIP
*/
#include "config.h"

int main() {

  // this shall be called periodically
  LL_RUN();

  // just call all to check if everything compiles
  unsigned char buf[100];

  int valid = LL_RXFULL();
  int len = LL_RXREAD(buf);
  valid = LL_TXEMPTY();
  LL_TXWRITE(buf, 10);
}
