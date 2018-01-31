/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl (martin@jopdesign.com), Rasmus Ulslev Pedersen
  License: Simplified BSD
*/

// TODOs
// 0. use a struct to work with the packet and not "raw" memory
// 1. one UDP packet over slip
// 2. UDP "reply" over slip
//
//
//



//*****************************************************************************
// RATIONALE
//*********************************************************************

// Rationale:
// The rationale in the stack is to focus on the 'MAX_BUF_NUM' buffers. Each
// buffer is of length 'MAX_BUF_SIZE' with type 'unsigned char' (same as 'uint8_t').
// There are structs for each layer in the stack (.., IP, UDP, ..). Each struct holds
// pointer(s) to places in the associated buffer. The buffer is not manipulated directly,
// but accessed with get/set functions that take a struct pointer as argument; this struct
// holds the addresses of the header and data. Both the buffers and the structs are statically
// allocated. In example, udp[2], ip[2], and buf[2] are connected by index 2.
//
// Toolchain:
// patmos-clang -O2 -mserialize=checksumwcet.pml checksumwcet.c
// platin wcet -i checksumwcet.pml -b a.out -e checksum --report
// checksum is the analysis entry point that would be inlined with -O2

#include <stdio.h>
#include <string.h> // memcpy
#include <time.h>   // 'clock_t' and 'clock()'

//*****************************************************************************
// FORWARD DECLARATIONS (only on a "need-to" basis)
//*********************************************************************

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

// Martin thinks those different pointers are not the best way
// to represent IP packets.
//  RUP->MS: Agree. Would you make a typedef for udp as well? So we can get
//   both to work at the same time.

// new proposal for using types (MS)
// we cannot safely use shorter fields due to
// possible different byte orders.

typedef struct ip_t
{
  unsigned long length;
  unsigned long id;
  unsigned long prot;
  unsigned long source;
  unsigned long destination;
} ip_t;

// And we need to take care of byteorder (with hton() and ntoh()).

// use as a start 32-bit arrays to ensure it is word aligned.
// shall use the MAX_* when agreed on.
static unsigned long packets[3][1600 / 4];

//*****************************************************************************
// CONSTANT PARAMETERS SECTION
//*****************************************************************************

// period length (ms). Can also be cycles if on 'bare metal'
#define PERIOD 100 // 1000 // must be less than 2,147,483,647

// used by 'int currenttimemillis()'
#define CLOCKS_PER_MSEC (CLOCKS_PER_SEC / 1000)

// one 'BUF' is one packet/message
// very important WCET parameter
#define MAX_BUF_NUM 4
// bytes in multiples of 4
// very important WCET parameter
#define MAX_BUF_SIZE 1600
// fixed, static buffers to enable WCET analysis
static unsigned char buf[MAX_BUF_NUM][MAX_BUF_SIZE]; ///4];
// length of each buffer (in bytes)
static unsigned char bufbytes[MAX_BUF_NUM];

// UDP PROTOCOL //
static udpstruct_t udp[MAX_BUF_NUM];

// IP PROTOCOL //
#define VER_MASK 0xF0  //111_0000
#define HDR_MASK 0x0F  //0000_1111
#define FLAGS_MASK 0x7 //0111
static ipstruct_t ip[MAX_BUF_NUM];

// temp
// to be changed when we also have an ethernet header
//static char* header = (char*) buf[0]; //// todo: check this cast

//*****************************************************************************
// UTILITIES SECTION
//*****************************************************************************

// GENERAL

// TIMER UTILITIES

// function for getting number of milliseconds since "system-epoc"
// used with waitfornextperiod

typedef struct atype_t
{
  int x;
} atype_t;

__attribute__((noinline)) int currenttimemillis()
{
  //atype_t atype;

  //volatile struct timeval timenow;
  //gettimeofday(&timenow, NULL);

  clock_t start_t;
  start_t = clock();
  int mstime = ((unsigned int)start_t / (unsigned int)CLOCKS_PER_MSEC);

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

// CHECKSUM UTILITIES

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

//*****************************************************************************
// UDP SECTION
//*****************************************************************************
//RFC 768
//User Datagram Header Format
//  0      7 8     15 16    23 24    31
//  +--------+--------+--------+--------+
//  |     Source      |   Destination   |
//  |      Port       |      Port       |
//  +--------+--------+--------+--------+
//  |                 |                 |
//  |     Length      |    Checksum     |
//  +--------+--------+--------+--------+
//  |
//  |          data octets ...
//  +---------------- ...
//
//Pseudo-header prepended UDP header for checksum purposes
//  0      7 8     15 16    23 24    31
//  +--------+--------+--------+--------+
//  |          source address           |
//  +--------+--------+--------+--------+
//  |        destination address        |
//  +--------+--------+--------+--------+
//  |  zero  |protocol|   UDP length    |
//  +--------+--------+--------+--------+

// UDP source port
int getudpsrcport(udpstruct_t *udp_p)
{
  return (udp_p->header[0] << 8) | (udp_p->header[1]);
}

void setudpsrcport(udpstruct_t *udp_p, int value)
{
  udp_p->header[0] = (unsigned char)(value >> 8);
  udp_p->header[1] = (unsigned char)value;
}

// UDP destination port
int getudpdstport(udpstruct_t *udp_p)
{
  return (udp_p->header[2] << 8) | (udp_p->header[3]);
}

void setudpdstport(udpstruct_t *udp_p, int value)
{
  udp_p->header[2] = (unsigned char)(value >> 8);
  udp_p->header[3] = (unsigned char)value;
}

// UDP length
int getudplen(udpstruct_t *udp_p)
{
  return (udp_p->header[4] << 8) | udp_p->header[5];
}

void setudplen(udpstruct_t *udp_p, int value)
{
  udp_p->header[4] = (unsigned char)(value >> 8);
  udp_p->header[5] = (unsigned char)value;
}

int getudpchksum(udpstruct_t *udp_p)
{
  return (udp_p->header[6] << 8) | udp_p->header[7];
}

void setudpchksum(udpstruct_t *udp_p, int value)
{
  udp_p->header[6] = (unsigned char)(value >> 8);
  udp_p->header[7] = (unsigned char)value;
}

// udp data
unsigned char *getudpdata(udpstruct_t *udp_p)
{
  return udp_p->data;
}

void setudpdata(udpstruct_t *udp_p, unsigned char *data, int datacount)
{
  udp_p->data = udp_p->header + 8 * (sizeof(unsigned char)); //udp header is 8 bytes
  //todo _Pragma
  for (int i = 0; i < datacount; i++)
  {
    udp_p->data[i] = data[i];
  }
}

// udp functions //

/* compiler error
// init udp header fields
void initudp(udpstruct_t *udp_p,
             int srcport,
             int dstport,//kickstart with sending a request to each of the other cores
  txmsg.reqid = 1; // want to get message 1 from another core (and no more)
  memcpy(&core[cid].tx[0], &txmsg, sizeof(txmsg));
             int len,
             int chksum, // to be calculated, call with 0xFFFF
             unsigned char *data,
             int datacount)
{
  setudpsrcport(udp_p, srcport);
  setudpdstport(udp_p, dstport);
  setudpdata(udp_p, data, datacount);
  setudplen(udp_p, len);
  //if(chksum == 0xFFFF) setudpchksum(udp_p, calculateudpchecksum(udp_p));
}
*/

//*****************************************************************************
// IP SECTION
//*****************************************************************************
// RFC 791:  Internet Header Format
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Version|  IHL  |Type of Service|          Total Length         |4500 0073    0.. 3
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         Identification        |Flags|      Fragment Offset    |0000 4000    4.. 7
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  Time to Live |    Protocol   |         Header Checksum       |4011 *b861*  8..11 (*10..11*)
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Source Address                          |c0a8 0001   12..16
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Destination Address                        |c0a8 00c7   16..19
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Options                    |    Padding    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//member __.Header with get()      = header and
//                      set(value) = header <- value

/* compile error
// Version 4-bits .[V0]
int getipver(ipstruct_t *ip_p)
{//kickstart with sending a request to each of the other cores
  txmsg.reqid = 1; // want to get message 1 from another core (and no more)
  memcpy(&core[cid].tx[0], &txmsg, sizeof(txmsg));
  return ip_p->header[0] >> 4;
}
*/

void setipver(ipstruct_t *ip_p, int value)
{
  ip_p->header[0] = (char)((value << 4) | (ip_p->header[0] & HDR_MASK));
}

// Header 4-bits .[0H], number of 32-bit words
int getiphdr(ipstruct_t *ip_p)
{
  return ip_p->header[0] & HDR_MASK;
}

void setiphdr(ipstruct_t *ip_p, int value)
{
  ip_p->header[0] = (char)(ip_p->header[0] & VER_MASK) | (value & HDR_MASK);
}

// Type of service 8-bits .[1]
int getiptos(ipstruct_t *ip_p)
{
  return ip_p->header[1];
}

void setiptos(ipstruct_t *ip_p, int value)
{
  ip_p->header[1] = (unsigned char)value;
}

// Total length in byte, 16-bits .[2] .[3]
int getiptlen(ipstruct_t *ip_p)
{
  return (ip_p->header[2] << 8) | ip_p->header[3];
}

void setiptlen(ipstruct_t *ip_p, int value)
{
  ip_p->header[2] = (unsigned char)(value >> 8);
  ip_p->header[3] = (unsigned char)value;
}

// Identification, 16-bits, .[4] .[5]
int getipid(ipstruct_t *ip_p)
{
  return (ip_p->header[4] << 8) | ip_p->header[5];
}

void setipid(ipstruct_t *ip_p, int value)
{
  // check >>> or >>
  ip_p->header[4] = (unsigned char)value >> 8;
  ip_p->header[5] = (unsigned char)value;
}

// Flags, 3-bits, .[6H]
int getflags(ipstruct_t *ip_p)
{
  return ip_p->header[6] >> 5;
}

void setipflags(ipstruct_t *ip_p, int value)
{
  ip_p->header[6] = (unsigned char)((value & FLAGS_MASK) << 5) | (ip_p->header[6] & 0x1F); //0001_1111
}

// Fragment offset, 13-bits, .[6L] .[7]
int getipfoff(ipstruct_t *ip_p)
{
  return ((ip_p->header[6] & 0x1F) << 8) | ip_p->header[7]; //0001_1111
}

void setipfoff(ipstruct_t *ip_p, int value)
{
  ip_p->header[6] = (unsigned char)(ip_p->header[6] & 0xE0) | (value >> 8);
  ip_p->header[7] = (unsigned char)value;
}

// Time to live, 8-bits, .[8]
int getipttl(ipstruct_t *ip_p)
{
  return ip_p->header[8];
}

void setipttl(ipstruct_t *ip_p, int value)
{
  ip_p->header[8] = (unsigned char)value;
}

// Protocol, 8-bits, .[9]
int getipprot(ipstruct_t *ip_p)
{
  return ip_p->header[9];
}

void setipprot(ipstruct_t *ip_p, int value)
{
  ip_p->header[9] = (unsigned char)value;
}

// Checksum, 16-bits, .[10] .[11]
int getiphchksum(ipstruct_t *ip_p)
{
  return (ip_p->header[10] << 8) | ip_p->header[11];
}

void setiphchksum(ipstruct_t *ip_p, int value)
{
  ip_p->header[10] = (unsigned char)(value >> 8);
  ip_p->header[11] = (unsigned char)value;
}

// Source IP, 32-bits, .[12] .[13] .[14] .[15]
int getipsrcip(ipstruct_t *ip_p)
{
  return (ip_p->header[12] << 24) | (ip_p->header[13] << 16) |
         (ip_p->header[14] << 8) | (ip_p->header[15]);
}

void setipsrcip(ipstruct_t *ip_p, int value)
{
  ip_p->header[12] = (unsigned char)(value >> 24); // check the shift
  ip_p->header[13] = (unsigned char)(value >> 16);
  ip_p->header[14] = (unsigned char)(value >> 8);
  ip_p->header[15] = (unsigned char)value;
}

// Destination IP, 32-bits, .[16] .[17] .[18] .[19]
//int getipdstip(ipstruct_t* ip_p) __attribute__((noinline));
__attribute__((noinline)) int getipdstip(ipstruct_t *ip_p)
{
  return (ip_p->header[16] << 24) | (ip_p->header[17] << 16) |
         (ip_p->header[18] << 8) | ip_p->header[19];
}

void setipdstip(ipstruct_t *ip_p, int value)
{
  ip_p->header[16] = (unsigned char)(value >> 24);
  ip_p->header[17] = (unsigned char)(value >> 16);
  ip_p->header[18] = (unsigned char)(value >> 8);
  ip_p->header[19] = (unsigned char)value;
}

// payload data (such as ICMP, UDP, or TCP message)
unsigned char *getipdata(ipstruct_t *ip_p)
{
  return ip_p->data;
}

void setipdata(ipstruct_t *ip_p, unsigned char *data)
{
  ip_p->data = data;
}

// ip functions //

// init ip packet fields
void initip(ipstruct_t *ip_p,
            int ver,
            int hdrwrds,
            int tos,
            int id,
            int flags,
            int foff,
            int ttl,
            int prot,
            int srcip,
            int dstip,
            //int* data,
            int tlen,
            int hchksum) // to be calculated:0xFFFF
{
  setipver(ip_p, ver);
  setiphdr(ip_p, hdrwrds);
  setiptos(ip_p, tos);
  setipid(ip_p, id);
  setipflags(ip_p, flags);
  setipfoff(ip_p, foff);
  setipttl(ip_p, ttl);
  setipprot(ip_p, prot);
  setipsrcip(ip_p, srcip);
  setipdstip(ip_p, dstip);
  setipdata(ip_p, ip_p->header + 20 * (sizeof(unsigned char)));
  setiptlen(ip_p, tlen);
  //if(hchksum == 0xFFFF) setiphchksum(ip_p, calculateipchecksum(ip_p));
}

// Data, byte array, such an an ICMP message
// Manually update TLen and Hchksum
// member __.Data with get ()     = data and
// set(value) = data <- value

//*****************************************************************************
// ETHERNET SECTION (martin?)
//*****************************************************************************

//todo (martin?): create a simple set of files for getting started
// split file?

//*****************************************************************************
// TEST SECTION: Temp. area to keep "ok" test code and misc. snippets
//*****************************************************************************

// TIMER TESTS //

int timer_test()
{
  int res = 1; // set test to "ok" to begin with
  // to see more, then set show to 1
  int show = 0;
  if (show)
    printf("timer_test!\n");
  if (show)
    printf("PERIOD: %d ms \n\n", PERIOD);
  //start timer
  initwaitfornextperiod();
  int before = currenttimemillis();
  if (show)
    printf("Timer before 'waitforoneperiod()':  %10d ms \n", before);
  int elapsed1 = waitfornextperiod();
  int after = currenttimemillis();
  if (show)
    printf("Timer after 'waitforoneperiod()':   %10d ms \n", after);
  if (show)
    printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed1);
  if (show)
    printf("Elapsed time since this wfnp call:  %10d ms \n\n", (after - before));
  // uncomment 'for'  loop to get a false test
  //for(volatile int i = 0; i < 100000000; i++); // 'volatile' to avoid '-O2' optimization
  wait(123); // 'wait(321)' will give a false test
  if (show)
    printf("Timer before 'waitforoneperiod()':  %10d ms \n", currenttimemillis());
  int elapsed2 = waitfornextperiod();
  // test result fail?
  if (elapsed2 != 123)
    res = 0;
  if (show)
    printf("Timer after 'waitforoneperiod()':   %10d ms \n", currenttimemillis());
  if (show)
    printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed2);
  //1 ms left
  wait(999);
  if (show)
    printf("Deadline (-1, so hurry up!):        %10d ms \n", deadline());
  //broke deadline
  waitfornextperiod();
  // disable if you want to skip the wait for 1 sec
  // wait(1001);
  if (deadline())
    if (show)
      printf("Deadline (+1, bye bye...):        %10d ms \n", deadline());

  if (show)
    printf("\ntimer_test result: %s\n", res ? "true" : "false");
  return res;
}

// CHECKSUM TESTS //

int checksum_test()
{
  int res = 1; //ok

  // Test 1: 0xb861 is the checksum
  //   0..3|Version|  IHL  |Type of Service|          Total Length         |4500 0073
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //   4..7|         Identification        |Flags|      Fragment Offset    |0000 4000
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //  8..11|  Time to Live |    Protocol   |         Header Checksum       |4011 *b861*
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // 12..16|                       Source Address                          |c0a8 0001
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // 16..19|                    Destination Address                        |c0a8 00c7
  unsigned char ipdata[] = {0x45, 0x00, 0x00, 0x73,
                            0x00, 0x00, 0x40, 0x00,
                            0x40, 0x11, 0xFF, 0xFF, // checksum  is 'set' to 0xFFFF
                            0xc0, 0xa8, 0x00, 0x01,
                            0xc0, 0xa8, 0x00, 0xc7};
  ip[0].header = ipdata;
  int checksum = calculateipchecksum(&ip[0]);
  //printf("checksum_test=%08x\n", checksum);
  if (checksum != 0xb861)
    res = 0; //failed test
  return res;
}

int mod_test()
{
  int res = 1; //test ok
  if (mod(10, 3) != 1)
    res = 0; // test fail
  return res;
}

// IP TESTS //

int test_initip()
{
  int show = 0;
  int res = 1; //test ok
  if (show)
    printf("start of test_initip\n");
  // ip test
  ip[0].header = buf[0]; //todo: check
  initip(&ip[0],
         4,          // ver
         5,          // hdrwrds
         0,          // tos
         0,          // id
         0,          // flags
         0,          // foff
         10,         // ttl
         0x11,       // prot, icmp:1, udp:17
         0x01020304, // srcip
         0x01020305, // dstip
                     // int* data
         20,         // tlen, todo:not correct
         0xFFFF);    // int hchksum  (0xFFFF will leave it alone)

  if (!(getiptlen(&ip[0]) == 20))
  {
    if (show)
      printf("Error: getiptlen(ip[0])==%d\n", getiptlen(&ip[0]));
    res = 0; // test fail
  }
  else
  {
    if (show)
      printf("Ok: getiptlen(ip[0])==%d\n", getiptlen(&ip[0]));
  }

  // with udp payload
  udp[0].header = ip[0].data;
/* link error because of compile error
  initudp(&udp[0],
          4321,                           // int srcport,
          1234,                           // int dstport,
          8 + 2,                          // int len,
          0xFFFF,                         // int chksum (will be calculated if called with 0xFFFF)
          (unsigned char[2]){0x62, 0x62}, // some data
          2);                             // how much data (in elements, i.e., char(s))

*/

  if (!(getudplen(&udp[0]) == 10))
  {
    if (show)
      printf("Error: getudplen(udp[0])==%d\n", getudplen(&udp[0]));
    res = 0; // test fail
  }
  else
  {
    if (show)
      printf("Ok: getudplen(udp[0])==%d\n", getudplen(&udp[0]));
  }
  return res;
}

// WCET "tests": just here to platin can reach the code from main
//void testwcet() __attribute__((noinline));
void testwcet()
{
  // 'volatile' is important so gcc does not optimize the function call
  // and then platin can't find it...
  volatile int foo = -1;
  foo = getipdstip(&ip[0]);
  foo = waitfornextperiod();
}

// final tests //

// insert new unittests here
int tests()
{
  int show = 1; // set to 1 if intermediate results are desired
  int res = 1;  //test ok
  // some tests
  if (!checksum_test())
  {
    if (show)
      printf("checksum_test() failed\n");
    res = 0; // test fail
  }
  if (!timer_test())
  {
    if (show)
      printf("timer_test() failed\n");
    res = 0; // test fail
  }
  if (!mod_test())
  {
    if (show)
      printf("mod_test() failed\n");
    res = 0; // test fail
  }
  if (!test_initip())
  {
    if (show)
      printf("test_initip() failed\n");
    res = 0; // test fail
  }
  return res;
}

//*****************************************************************************
// MAIN SECTION: Place various "main" functions here and enable as needed
//*****************************************************************************

// Basic testing
int main()
{
  printf("Hello tpip world! \n\n");
  if (!tests())
    printf("testing: one or more unit tests failed\n\n");
  else
    printf("testing: all tests ok\n\n");

  // temp ip test
  //test_initip();
  testwcet();
  //int foobar = bar + bar;
  //printf("\n", foobar);

  // 4500 0073 0000 4000 4011 *b861* c0a8 0001 c0a8 00c7
  // gives 0x0000 if 0xb861 is added in
  //int sum                   = 0x4500 + 0x0073 + 0x0000 + 0x4000 + 0x4011 +
  //                            0xc0a8 + 0x0001 + //0xB861 +
  //                            0xc0a8 + 0x00c7;
  //printf("sum               = 0x%08X\n", sum);
  //int carry                 = (sum & 0xFFFF0000) >> 16;
  //printf("carry             = 0x%08X\n", carry);
  //int sum16                 = sum & 0x0000FFFF;
  //printf("sum16             = 0x%08X\n", sum16);
  //int sum16pluscarry        = sum16 + carry;
  //printf("sum16pluscarry    = 0x%08X\n", sum16pluscarry);
  //int checksum              = (~sum16pluscarry) & 0x0000FFFF;
  //printf("checksum '0xb861' = 0x%08X\n", checksum);

  // mod call that only works with -O0 and not with -O2
  // when running make wcet
  //printf("mod(3,2)=%d", mod(3,2));

  //unsigned char a = 0x80;
  //unsigned char b = 0x01;
  //int aplusb = a + b;
  //printf("m1)sum=0x%x\n",aplusb);

  // dump to file
  unsigned char pcapdata[2048];
  unsigned char *pcapdataptr = pcapdata;

  // global header
  //typedef struct pcap_hdr_s {
  //  guint32 magic_number;   /* magic number */
  //  guint16 version_major;  /* major version number */
  //  guint16 version_minor;  /* minor version number */
  //  gint32  thiszone;       /* GMT to local correction */
  //  guint32 sigfigs;        /* accuracy of timestamps */
  //  guint32 snaplen;        /* max length of captured packets, in octets */
  //  guint32 network;        /* data link type */
  //} pcap_hdr_t;

  unsigned int magic_number = 0xa1b2c3d4;
  memcpy(pcapdataptr, &magic_number, sizeof(magic_number));
  pcapdataptr += sizeof(magic_number);
  unsigned short version_major = 0x2;
  memcpy(pcapdataptr, &version_major, sizeof(version_major));
  pcapdataptr += sizeof(version_major);
  unsigned short version_minor = 0x4;
  memcpy(pcapdataptr, &version_minor, sizeof(version_minor));
  pcapdataptr += sizeof(version_minor);
  int thiszone = -3600;
  memcpy(pcapdataptr, &thiszone, sizeof(thiszone));
  pcapdataptr += sizeof(thiszone);
  unsigned int sigfigs = 0;
  memcpy(pcapdataptr, &sigfigs, sizeof(sigfigs));
  pcapdataptr += sizeof(sigfigs);
  unsigned int snaplen = 65535;
  memcpy(pcapdataptr, &snaplen, sizeof(snaplen));
  pcapdataptr += sizeof(snaplen);
  unsigned int network = 1;
  memcpy(pcapdataptr, &network, sizeof(network));
  pcapdataptr += sizeof(network);

  // packet header
  //typedef struct pcaprec_hdr_s {
  //  guint32 ts_sec;         /* timestamp seconds */
  //  guint32 ts_usec;        /* timestamp microseconds */
  //  guint32 incl_len;       /* number of octets of packet saved in file */
  //  guint32 orig_len;       /* actual length of packet */
  //} pcaprec_hdr_t;
  unsigned int ts_sec = time(NULL);
  memcpy(pcapdataptr, &ts_sec, sizeof(ts_sec));
  pcapdataptr += sizeof(ts_sec);
  unsigned int ts_usec = 0;
  memcpy(pcapdataptr, &ts_usec, sizeof(ts_usec));
  pcapdataptr += sizeof(ts_usec);
  unsigned int incl_len = 14 + 20;
  memcpy(pcapdataptr, &incl_len, sizeof(incl_len));
  pcapdataptr += sizeof(incl_len);
  unsigned int orig_len = incl_len;
  memcpy(pcapdataptr, &orig_len, sizeof(orig_len));
  pcapdataptr += sizeof(orig_len);

  // ethernet
  long long unsigned int dstethaddr = 0x010203040506;
  memcpy(pcapdataptr, &dstethaddr, sizeof(dstethaddr));
  pcapdataptr += sizeof(dstethaddr);
  long long unsigned int srcethaddr = 0x020304050607;
  memcpy(pcapdataptr, &srcethaddr, sizeof(srcethaddr));
  pcapdataptr += sizeof(srcethaddr);
  unsigned short etype = 0x0800; //0x0800 IPv4
  memcpy(pcapdataptr, &etype, sizeof(etype));
  pcapdataptr += sizeof(etype);

  //IP v4
  // RFC 791:  Internet Header Format
  //  0                   1                   2                   3
  //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |Version|  IHL  |Type of Service|          Total Length         |4500 0073    0.. 3
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |         Identification        |Flags|      Fragment Offset    |0000 4000    4.. 7
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |  Time to Live |    Protocol   |         Header Checksum       |4011 *b861*  8..11 (*10..11*)
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                       Source Address                          |c0a8 0001   12..16
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                    Destination Address                        |c0a8 00c7   16..19
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  // |                    Options                    |    Padding    |
  // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  //initip(&ip[0],
  unsigned char verihl = 0x45;
  memcpy(pcapdataptr, &verihl, sizeof(verihl));
  pcapdataptr += sizeof(verihl);
  unsigned char tos = 0x00;
  memcpy(pcapdataptr, &tos, sizeof(tos));
  pcapdataptr += sizeof(tos);
  unsigned short tlen = 20;
  memcpy(pcapdataptr, &tlen, sizeof(tlen));
  pcapdataptr += sizeof(tlen);

  unsigned short id = 0x0001;
  memcpy(pcapdataptr, &id, sizeof(id));
  pcapdataptr += sizeof(id);
  unsigned short ff = 0x0000; // flags and fragment offset
  memcpy(pcapdataptr, &ff, sizeof(ff));
  pcapdataptr += sizeof(ff);

  unsigned char ttl = 10;
  memcpy(pcapdataptr, &ttl, sizeof(ttl));
  pcapdataptr += sizeof(ttl);
  unsigned char prot = 0x11; // prot, icmp:1, udp:0x11
  memcpy(pcapdataptr, &prot, sizeof(prot));
  pcapdataptr += sizeof(prot);
  unsigned short hcksum = 0x0000;
  memcpy(pcapdataptr, &hcksum, sizeof(hcksum));
  pcapdataptr += sizeof(hcksum);

  unsigned int srcip = 0x01020304; // srcip
  memcpy(pcapdataptr, &srcip, sizeof(srcip));
  pcapdataptr += sizeof(srcip);

  unsigned int dstip = 0x01020305; // dstip
  memcpy(pcapdataptr, &dstip, sizeof(dstip));
  pcapdataptr += sizeof(dstip);

  int size = pcapdataptr - pcapdata;
  printf("m0) size=%d", size);

  FILE *datafile = fopen("tpip.pcap", "wb");
  fwrite(pcapdata, size, 1, datafile);
  fclose(datafile);
}

//*****************************************************************************
// TEMP SECTION: Misc. info for temp. usage
//*****************************************************************************
// size of types (and how to print them)
// (delete when sure about coding style)
// Type and printf     Storage size   Value range
// char %c             1 byte        -128 to 127 or 0 to 255
// unsigned char       1 byte         0 to 255
// signed char         1 byte        -128 to 127
// int %d              4 bytes       -2,147,483,648 to 2,147,483,647
// unsigned int %u     4 bytes        0 to 4,294,967,295
// short               2 bytes       -32,768 to 32,767
// unsigned short      2 bytes        0 to 65,535
// long                4 bytes       -2,147,483,648 to 2,147,483,647
// unsigned long       4 bytes        0 to 4,294,967,295
// long long int %lld  8 bytes        ...

//RFC 792: ICMP
//Echo or Echo Reply Message
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |     Type      |     Code      |          Checksum             |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |           Identifier          |        Sequence Number        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |     Data ...
//   +-+-+-+-+-

//Code review

//1. Why not having the IP and UDP fields as part of the struct?
//https://stackoverflow.com/questions/15612488/what-good-is-a-function-pointer-inside-a-struct-in-c

//2. Char for the buffer is kind of ok. But word access is more efficient (Do er care?). Anyway, in C one can cast anything to anything. It is just memory. But the words must be aligned on word boundary for Patmos.

//3. There are no connections between your array of IP structs and your buffers.

//4. waitfornextperiod contains a classic mistake as it waits for some time and does not increments a variable with the period. See as a solution: https://github.com/t-crest/iot-rt/blob/master/src/main/scala/rtapi/RtThread.scala Essentioal is line 37

//5. checksum: you could also pad the buffer with missing zeros and have a single loop. When done unconditinally then the buffer needs to be a little bit longer (3 bytes). [...]

//6. UDP field code: with a struct access to the indicidual fields is easier. Maybe not even needing functions for this.

//7. Same for IP.
