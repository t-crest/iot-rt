#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifndef __patmos__
#include <termios.h>
#else
#include <machine/patmos.h>
#endif

#include "slip.h"
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
static int slipoutcnt = 0;

static unsigned char txbuf[MAX];
// TODO some counters and flags for TX

int tpip_slip_init(char *str) {

#ifdef __patmos__
  
#endif
#ifndef __patmos__
  // O_NDELAY returns 0 when no character available, useful for polling
  fd = open(str, O_RDONLY | O_NDELAY);
  printf("SLIP open: %s %d\n", str, fd);
#endif
  return 0;
}


int tpip_slip_getchar(unsigned char *cptr) {
#ifdef __patmos__
  volatile _IODEV int *uart2_ptr = (volatile _IODEV int *)0xF00e0004;
  volatile _IODEV int *uart2_status_ptr = (volatile _IODEV int *)0xF00e0000;

  if (((*(uart2_status_ptr)) & 0x02) != 0) {
    *cptr = *uart2_ptr;
    return 1;
  } else {
    return 0;
  }
#endif
#ifndef __patmos__
  return read(fd, cptr, 1);
#endif
}

void tpip_slip_putchar(unsigned char c) {
  // TODO send one character
#ifdef __patmos__
    volatile _IODEV int *uart2_ptr = (volatile _IODEV int *)0xF00e0004;
    volatile _IODEV int *uart2_status_ptr = (volatile _IODEV int *)0xF00e0000;
    // MS: should try once and not have a busy wait here
    while (((*(uart2_status_ptr)) & 0x01) == 0); // busy wait
    *uart2_ptr = c;
    //printf("%02d:tpip_slip_putchar(0x%02x)\n", slipoutcnt, c);
    slipoutcnt++;
#endif
}

int tpip_slip_is_end(unsigned char c){
  if (c == END)
    return 1;
  else
    return 0;
}

int tpip_slip_is_esc(unsigned char c){
  if (c == ESC)
    return 1;
  else
    return 0;
}

int tpip_slip_was_esc(unsigned char c){
  if (c == ESC_ESC)
    return ESC;
  else if (c == ESC_END)
    return END;
  else {
    printf("slip esc error: 0x%02x\n", c);
    return c;
  } 
}

void tpip_slip_put_end(){
  tpip_slip_putchar(END);
}

void tpip_slip_put_esc(){
  tpip_slip_putchar(ESC);
}

void tpip_slip_put_esc_esc(){
  tpip_slip_putchar(ESC_ESC);
}

void tpip_slip_put_esc_end(){
  tpip_slip_putchar(ESC_END);
}

// peridic stuff
// TODO: sometimes a wrong end of packet detection. Worked in the non-polling mode :-()
// TODO: should also contain the sending
// noline for WCET analysis
__attribute__((noinline)) void tpip_slip_run();

void tpip_slip_run() {

  unsigned char c;

  // MS: why is this loop not recognized by platin?
//  _Pragma("loopbound min 4 max 4")
//  for(int i=0; i<4; ++i) {
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
//  }
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

// slip
// SLIP sending:
//   if SLIP_ESC 0xDB is in the datagram then these two bytes are sent 
//     SLIP_ESC 0xDB -> SLIP_ESC 0xDB + SLIP_ESC_ESC 0xDD
//   if SLIP_END 0xC0 is in the datagram then two bytes are sent 
//     SLIP_END 0xC0 -> SLIP_ESC 0xDB + SLIP_ESC_END 0xDC
//   finally append an END byte 
//     SLIP_END 0xC0
// SLIP receiving:
//   if SLIP_ESC 0xDB is received then the next byte is either  
//     0xDC (SLIP_ESC_END); really meaning 0xC0
//       0xDB + 0xDC -> 0xC0
//     0xDD (SLIP_ESC_ESC); really meaning 0xDB
//       0xDB + 0xDD -> 0xDD
//   done when the END byte is detected
//     SLIP_END 0xC0; stop
#ifndef __patmos__
int set_interface_attribs(int fd, int speed)
{
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0)
  {
    printf("Error from tcgetattr: %s\n", strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, (speed_t)speed);
  cfsetispeed(&tty, (speed_t)speed);

  tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;      /* 8-bit characters */
  tty.c_cflag &= ~PARENB;  /* no parity bit */
  tty.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  /* fetch bytes as they become available */
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 1;

  if (tcsetattr(fd, TCSANOW, &tty) != 0)
  {
    printf("Error from tcsetattr: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

void set_mincount(int fd, int mcount)
{
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0)
  {
    printf("Error tcgetattr: %s\n", strerror(errno));
    return;
  }

  tty.c_cc[VMIN] = mcount ? 1 : 0;
  tty.c_cc[VTIME] = 5; /* half second timer */

  if (tcsetattr(fd, TCSANOW, &tty) < 0)
    printf("Error tcsetattr: %s\n", strerror(errno));
}

static int fd;
int initserial()
{
  char *portname = "/dev/ttyUSB1";

  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0)
  {
    printf("Error opening %s: %s\n", portname, strerror(errno));
    return -1;
  }
  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  set_interface_attribs(fd, B115200);
  set_mincount(fd, 0);                /* set to pure timed read */

  return 1;
}

// returns number of bytes received when SLIP_END is received
int serialreceive(unsigned char *bufin, int max)
{
  int wlen;
 
  int rdlenall = 0;
  unsigned long words_to_get;
  unsigned char *bufintmp = bufin;

  /* simple noncanonical input */
  unsigned char c = 0x00;
  do
  {
    int rdlen;
    unsigned char buf[2000];
    rdlen = read(fd, buf, sizeof(buf) - 1);
    if (rdlen > 0)
    {
      unsigned char *p;
      printf("Read %2d bytes from serial\n", rdlen);
      int i = 0;
      for (p = buf; rdlen-- > 0; p++)
      {
        printf("%02d: 0x%02x\n", i, *p);
        i++;
        *bufin = *p;
        c = *p;
        bufin++;
      }
      //printf("\n");
    }
    else if (rdlen < 0)
    {
      printf("Error from read: %d: %s\n", rdlen, strerror(errno));
    }
    /* repeat read to get full message */
    rdlenall += rdlen;
  } while (c != END && rdlenall <= max);

  printf("serial received %d \"slip\" bytes:\n", rdlenall);
  
  // i is slip, j is data
  int j =0;
  for(int i = 0; i < rdlenall-1; i++)
  {
      unsigned char c = *(bufintmp+i);
      if (c == ESC) {
        c = *(bufintmp + (i++));
        if(c == ESC_END) {
          c = END;  
        } 
        else if(c == ESC_ESC) {
          c = ESC_ESC;
        } else {
          printf("slip error on receive index %d, expected ESC_END or ESC_ESC but got 0x%02x\n",
                 i, c);
        }
      }
      
      *(bufintmp+j) = c;
      j++;
  }
  // clear SLIP_END
  *(bufintmp+j) = 0x00;

  printf("\n");
  printf("serial received data %d bytes:\n", j);
 

  return j;
}

// it appends SLIP_END as the last byte
int serialsend(unsigned char *bufin, int cnt)
{
  int wlen;
  for(int i=0; i < cnt; i++, bufin++){
    wlen = write(fd, bufin, 1);
    if (wlen != 1) printf("Error from write: %d, %d\n", wlen, errno);
    for(int i = 0; i < 100; i++)
      tcdrain(fd);
  }
  static unsigned char endslip = END;
  wlen = write(fd, &endslip, 1);
  tcdrain(fd);
  if (wlen != 1) printf("Error from write: %d, %d\n", wlen, errno);
  for(int i = 0; i < 100; i++)
    tcdrain(fd);

  return wlen;
}
#endif
