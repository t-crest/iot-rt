// tpiputil.c

//#include <arpa/inet.h>
#include "tpiputil.h"

// // CHECKSUM UTILITIES (on the original structs, so that is to be updated)

//typedef struct udpstruct_t udpstruct_t;
typedef struct ipstruct_t ipstruct_t;
typedef struct udpstruct_t
{
  ipstruct_t *ipstructp;
  unsigned char *header; // also the ip "data"
  unsigned char *data;   // this is the udp data
} udpstruct_t;

typedef struct ipstruct_t
{
  unsigned char *header;
  unsigned char *data; // this is the udp header
} ipstruct_t;

// 16-bit (pairs of two byte array entries)
int datasum(char data[], int arraysize)
{
  //printf("arraysize=%d\n", arraysize);
  for (int i = 0; i < arraysize; i = i + 1)
  {
    //printf("...data[%d]=0x%X\n", i, data[i]);
  }
  int datachecksum = 0;
  if (arraysize == 0)
  {
    // nothing to do
  }
  else if (arraysize == 1)
  {
    datachecksum = ((int)data[0] << 8); // Pad with a "zero"
  }
  else
  {
    for (int i = 0; i < arraysize - 1; i = i + 2)
    { // byte "pairs"
      datachecksum = datachecksum + (((int)data[i] << 8) | ((int)data[i + 1]));
      //printf("datachecksum=0x%X\n", datachecksum);
    }
    if (arraysize % 2 != 0) // one byte left
      datachecksum = datachecksum + ((int)data[arraysize - 1] << 8);
  }

  return datachecksum;
}

// ip header checksum calculation
int calculateipchecksum(ipstruct_t *ip_p) __attribute__((noinline));
int calculateipchecksum(ipstruct_t *ip_p)
{
  unsigned char *h = ip_p->header;
  int checksum = ((h[0] << 8) + h[1]) + ((h[2] << 8) + h[3]) +
                 ((h[4] << 8) + h[5]) + ((h[6] << 8) + h[7]) +
                 ((h[8] << 8) + h[9]) + // ignore old checksum: (h[10]<<8)+h[11]
                 ((h[12] << 8) + h[13]) + ((h[14] << 8) + h[15]) +
                 ((h[16] << 8) + h[17]) + ((h[18] << 8) + h[19]);
  if ((checksum & 0xFFFF0000) > 0)
    checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
  if ((checksum & 0xFFFF0000) > 0)
    checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
  if ((checksum & 0xFFFF0000) > 0)
    checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
  checksum = (~checksum) & 0x0000FFFF;
  return checksum;
}

// TIMER UTILITIES

// function for getting number of milliseconds since "system-epoc"
// used with waitfornextperiod

__attribute__((noinline)) int currenttimemillis()
{
  //atype_t atype;

  //volatile struct timeval timenow;
  //gettimeofday(&timenow, NULL);

  clock_t start_t;
  start_t = clock();
  int mstime = ((unsigned int)start_t / (unsigned int) CLOCKS_PER_MSEC);

  //volatile long long int msec = 0;
  //msec = timenow.tv_sec;
  //msec = msec + ((/*long long*/ int) timenow.tv_sec) ;//* 1000LL;
  //msec = msec + atype.x; //((long long int) timenow.tv_sec) ;//* 1000LL;
  //msec = msec  + (long long int) timenow.tv_usec / 1000LL;
  // push "epoc" from 1970 to now - approx. 277 hrs
  //   to having 'long long int' everywhere
  //msec = msec - 1511e9;
  return mstime;
}

//spins for a number of ms
//used by waitfornextperiod
//note: similar to 'clock()' in time.h
void wait(int waittime) __attribute__((noinline));
void wait(int waittime)
{
  int now = currenttimemillis();
  _Pragma("loopbound min 0 max 256") while (currenttimemillis() - now < waittime);
}

// wcet version of 'a % b'. '%' platin did not like '%'
int mod(int a, int b) __attribute__((noinline));
int mod(int a, int b)
{
  return a - (b * (a / b));
}

// init waitfornextperiod
static volatile int _start = -1; //Is 'volatile' needed here?
void initwaitfornextperiod()
{
  _start = currenttimemillis();
}

// will block (by spinning) until next period
// returns time since 'initwaitfornextperiod()' was called
//   so not including the rest of the PERIOD
// if return value is greater then PERIOD, it might indicate a deadline problem
//todo: validate/check with ms scala code
__attribute__((noinline)) int waitfornextperiod()
{
  volatile int totalwait = currenttimemillis() - _start; //
  int waitfor = PERIOD - ((currenttimemillis() - _start) % PERIOD);
  wait(waitfor);
  _start = currenttimemillis();
  return totalwait;
}

// if positive, the deadline is broke
//   (more than one PERIOD has elapsed since last 'waitfornextperiod() call)
int deadline()
{
  return (currenttimemillis() - _start) - PERIOD;
}