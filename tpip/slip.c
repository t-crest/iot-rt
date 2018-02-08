#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"

#define END 0xc0
#define ESC 0xdb
#define ESC_END 0xdc
#define ESC_ESC 0xdd

#define MAX 2000

static int fd;
static unsigned char rxbuf[MAX];
static int is_esc = 0;
static int cnt = 0;
static int rxfull = 0;

static unsigned char txbuf[MAX];
// TODO some counters and flags for TX

int tpip_slip_init(char *str) {

#ifdef __patmos__
  // no init needed
#endif
#ifndef __patmos__
  // O_NDELAY returns 0 when no character available, useful for polling
  fd = open(str, O_RDONLY | O_NDELAY);
  printf("SLIP open: %s %d\n", str, fd);
#endif
  return 0;
}


int tpip_slip_getchar(char *cptr) {
#ifdef __patmos__
  TODO: UART reading
#endif
#ifndef __patmos__
  return read(fd, cptr, 1);
#endif
}

void tpip_slip_putchar(char c) {
  // TODO send one character
}

// peridic stuff
// TODO: sometimes a wrong end of packet detection. Worked in the non-polling mode :-()
// TODO: should also contain the sending
void tpip_slip_run() {

    unsigned char c;

    for(int i=0; i<4; ++i) {
      if(tpip_slip_getchar(&c) == 1) {   
        if (is_esc) {
          if (c == ESC_ESC) {
            rxbuf[cnt++] = ESC;
          } else if (c == ESC_END) {
            rxbuf[cnt++] = END;
          }
          is_esc = 0;
        } else if (c == ESC) {
          is_esc = 1;
        } else if (c == END) {
          rxfull = 1;
        } else {
          rxbuf[cnt++] = c;
        }
        if (cnt == 2000) cnt = 0;
      }
    }
}

int tpip_slip_rxfull() {
  return rxfull;
}

int tpip_slip_rxread(unsigned char buf[]) {
 
  int i;

  for (i=0; i<cnt; ++i) {
    buf[i] = rxbuf[i];
  }
  rxfull = 0;
  cnt = 0;
  return i;
}

int tpip_slip_txempty() {
  return 0;
}

void tpip_slip_txwrite(unsigned char buf[], int len) {

}