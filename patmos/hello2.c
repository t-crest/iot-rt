/*

   UART2 test program, see README.md.

    Author: Martin Schoeberl
    Copyright: DTU, BSD License
*/

#include <machine/patmos.h>

int main() {

  volatile _IODEV int *uart2_ptr = (volatile _IODEV int *) 0xF00e0004;
  volatile _IODEV int *uart2_status_ptr = (volatile _IODEV int *) 0xF00e0000;
  char hello[] = "Hello Second World\r\n";

  for (int i = 0; hello[i] != 0; ++i) {
    while (((*(uart2_status_ptr)) & 0x01) == 0) ; // busy wait
    *uart2_ptr = hello[i]; 
  }  
}
