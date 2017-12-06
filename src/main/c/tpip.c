/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl, Rasmus Ulslev Pedersen
  License: Simplified BSD
*/

//patmos-clang -O2 -mserialize=checksumwcet.pml checksumwcet.c
//platin wcet -i checksumwcet.pml -b a.out -e checksum --report
// checksum is the analysis entry point that would be inlined with -O2

#include <stdio.h>
#include <sys/time.h>

//*****************************************************************************
// CONSTANT PARAMETERS SECTION
//*****************************************************************************

// period length (ms). Can also be cycles if on 'bare metal'
#define PERIOD 1000 // must be less than 2,147,483,647

// one 'BUF' is one packet/message
// very important WCET parameter
#define MAX_BUF_NUM 4
// bytes in multiples of 4
// very important WCET parameter
#define MAX_BUF_SIZE 1024
// fixed, static buffers to enable WCET analysis
// buffers (in 32-bit words)
static int buf[MAX_BUF_NUM][MAX_BUF_SIZE/4];
// length of each buffer (in bytes)
static int bufbytes[MAX_BUF_NUM];

// IP PROTOCOL CONSTANTS //
#define VER_MASK 0xF0  //111_0000
#define HDR_MASK 0x0F  //0000_1111
#define FLAGS_MASK 0x7 //0111

//*****************************************************************************
// FORWARD DECLARATIONS (only on a "need-to" basis)
//*********************************************************************

// temp
static int* header = buf[0];

//*****************************************************************************
// UTILITIES SECTION
//*****************************************************************************

// GENERAL

// TIMER UTILITIES

// function for getting number of milliseconds since "system-epoc"
// used with waitfornextperiod
int currenttimemillis() __attribute__((noinline));
int currenttimemillis(){
  struct timeval timenow;
  gettimeofday(&timenow, NULL);
  long long int msec = ((long long int) timenow.tv_sec) * 1000LL
    + (long long int) timenow.tv_usec / 1000LL;
  // push "epoc" from 1970 to now - approx. 277 hrs
  //   to having 'long long int' everywhere
  msec = msec - 1511e9;
  return (int)msec;
}

//spins for a number of ms
//used by waitfornextperiod
//note: similar to 'clock()' in time.h
void wait(int waittime) __attribute__((noinline));
void wait(int waittime){
  int now = currenttimemillis();
  _Pragma( "loopbound min 0 max 256" )
  while(currenttimemillis()-now < waittime);
}

// wcet version of 'a % b'. '%' platin did not like '%'
int mod(int a, int b) __attribute__((noinline));
int mod(int a, int b){
  return a - (b * (a/b));
}

// init waitfornextperiod
static volatile int _start = -1; //Is 'volatile' needed here?
void initwaitfornextperiod(){
  _start = currenttimemillis();
}

// will block (by spinning) until next period
// returns time since 'initwaitfornextperiod()' was called
//   so not including the rest of the PERIOD
// if return value is greater then PERIOD, it might indicate a deadline problem
//todo: validate/check with ms scala code
int waitfornextperiod(){
  int totalwait = currenttimemillis() - _start;
  int waitfor = PERIOD - ((currenttimemillis() - _start) % PERIOD);
  wait(waitfor);
  _start = currenttimemillis();
  return totalwait;
}

// if positive, the deadline is broke
//   (more than one PERIOD has elapsed since last 'waitfornextperiod() call)
int deadline(){
  return (currenttimemillis() - _start) - PERIOD;
}

// CHECKSUM UTILITIES

// 16-bit (pairs of two byte array entries)
int datasum(char data[], int arraysize){
  //printf("arraysize=%d\n", arraysize);
  for(int i = 0; i < arraysize; i = i + 1){
    //printf("...data[%d]=0x%X\n", i, data[i]);
  }

  int datachecksum = 0;
  if (arraysize == 0) {
    // nothing to do
  }
  else if (arraysize == 1){
    datachecksum = ((int)data[0] << 8); // Pad with a "zero"
  }
  else{
    for(int i = 0; i < arraysize-1; i = i + 2){ // byte "pairs"
      datachecksum = datachecksum + (((int) data[i] << 8) | ((int) data[i+1]));
      //printf("datachecksum=0x%X\n", datachecksum);
    }
    if (arraysize % 2 != 0)  // one byte left
      datachecksum = datachecksum + ((int) data[arraysize - 1] << 8);
  }

  return datachecksum;
}

// adds in the potntial carries and finally inverts
int checksum(int chksumcarry) __attribute__((noinline));
int checksum(int chksumcarry) {
  int checksum = chksumcarry;
  if ((checksum & 0xFFFF0000) > 0)
    checksum = (checksum > 16) + (checksum & 0x0000FFFF);
  if ((checksum & 0xFFFF0000) > 0)
    checksum = (checksum > 16) + (checksum & 0x0000FFFF);
  if ((checksum & 0xFFFF0000) > 0)
    checksum = (checksum > 16) + (checksum & 0x0000FFFF);
  checksum = ~checksum;
  return checksum & 0x0000FFFF;
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
//todo: create getter/setter functions for fields
//todo: checksum

// UDP source port
int getudpsrcport() {
  return (header[0] << 8) | (header[1]);
}

void setudpsrcport(int value){
  header[0] = (char) (value >> 8);
  header[1] = (char) value;
}

// UDP destination port
int getudpdstport() {
  return (header[2] << 8) | (header[3]);
}

void setudpdstport(int value){
  header[2] = (char) (value >> 8); 
  header[3] = (char) value;
}

// UDP length
int getudplen(){
  return (header[4] << 8) | header[5];
}                     

void setudplen(int value){
  header[4] = (char) (value >> 8);
  header[5] = (char) value;
} 

int getudpchksum(){
   return (header[6] << 8) | header[7];
}     

void setudpchksum(int value){
  header[6] = (char) (value >> 8);
  header[7] = (char) value;
} 

//*****************************************************************************
// IP SECTION
//*****************************************************************************
// RFC 791:  Internet Header Format
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Version|  IHL  |Type of Service|          Total Length         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         Identification        |Flags|      Fragment Offset    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  Time to Live |    Protocol   |         Header Checksum       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Source Address                          |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Destination Address                        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Options                    |    Padding    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 
//todo: create getter/setter functions for fields
//todo:checksum
//member __.Header with get()      = header and
//                      set(value) = header <- value

// Version 4-bits .[V0]
int getipver(){
  return header[0] >> 4;
}

void setipver(int value){
  header[0] = (char) ((value << 4) | (header[0] & HDR_MASK));
}

// Header 4-bits .[0H], number of 32-bit words
int getiphdr(){
  return header[0] & HDR_MASK;
}

void setiphdr(int value){
  header[0] = (char) (header[0] & VER_MASK) | (value & HDR_MASK);
}

// Type of service 8-bits .[1]
int getiptos(){
  return header[1];
}

void setiptos(int value){
  header[1] = (char) value;
}

// Total length in byte, 16-bits .[2] .[3]
int getiptlen(){
  return (header[2] << 8) | header[3];
}

void setiptlen(int value){
  header[2] = (char) (value >> 8);
  header[3] = (char) value;
}

// Identification, 16-bits, .[4] .[5]
int getipid(){
  return (header[4] << 8) | header[5];
}

void setipid(int value){
  // check >>> or >>
  header[4] = (char) value >> 8;
  header[5] = (char) value;
}

// Flags, 3-bits, .[6H]
int getflags(){
  return header[6] >> 5;
}

void setipflags(int value){
  header[6] = (char) ((value & FLAGS_MASK) << 5) | (header[6] & 0x1F); //0001_1111
}

// Fragment offset, 13-bits, .[6L] .[7]
int getipfoff(){
  return ((header[6] & 0x1F) << 8) | header[7]; //0001_1111
}

void setipfoff(int value){
  header[6] = (char) (header[6] & 0xE0) | (value >> 8);
  header[7] = (char) value;
}

// Time to live, 8-bits, .[8]
int getipttl() {
  return header[8];
}

void setipttl(int value){
  header[8] = (char) value;
}

// Protocol, 8-bits, .[9]
int getipprot() {
  return header[9];
} 
                      
void setprot(int value){
  header[9] = (char) value;
}

// Checksum, 16-bits, .[10] .[11]
int getiphchksum() {
  return (header[10] << 8) | header[11];
}

void setiphchksum(int value){
  header[10] = (char) (value >> 8);
  header[11] = (char) value;
}

// Source IP, 32-bits, .[12] .[13] .[14] .[15]
int getipsrcip () {
  return (header[12] << 24) | (header[13] << 16) |
          (header[14] <<  8) | (header[15]);
}                    

void setipsrcip(int value){
  header[12] = (char) (value >> 24); // check the shift
  header[13] = (char) (value >> 16);
  header[14] = (char) (value >> 8);
  header[15] = (char) value;
} 

// Destination IP, 32-bits, .[16] .[17] .[18] .[19]
int getipdstip(){
  return (header[16] << 24) | (header[17] << 16) |
         (header[18] <<  8) | header[19];  
}                    

void setipdstip(int value) {
  header[16] = (char) (value >> 24);
  header[17] = (char) (value >> 16);
  header[18] = (char) (value >> 8);
  header[19] = (char) value;
}

/// Data, byte array, such an an ICMP message
/// Manually update TLen and Hchksum
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

int timer_test() {
  int res = 0;
  // to see more, then set show to 1
  int show = 0;
  if(show) printf("timer_test!\n");
  if(show) printf("PERIOD: %d ms \n\n", PERIOD);
  //start timer
  initwaitfornextperiod();
  int before = currenttimemillis();
  if(show) printf("Timer before 'waitforoneperiod()':  %10d ms \n", before);
  int elapsed1 = waitfornextperiod();
  int after = currenttimemillis();
  if(show) printf("Timer after 'waitforoneperiod()':   %10d ms \n", after);
  if(show) printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed1);
  if(show) printf("Elapsed time since this wfnp call:  %10d ms \n\n", (after-before));
  // uncomment 'for'  loop to get a false test
  //for(volatile int i = 0; i < 100000000; i++); // 'volatile' to avoid '-O2' optimization
  wait(123); // 'wait(321)' will give a false test
  if(show) printf("Timer before 'waitforoneperiod()':  %10d ms \n", currenttimemillis());
  int elapsed2 = waitfornextperiod();
  if(show) printf("Timer after 'waitforoneperiod()':   %10d ms \n", currenttimemillis());
  if(show) printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed2);
  //1 ms left
  wait(999);
  if(show) printf("Deadline (-1, so hurry up!):        %10d ms \n", deadline());
  //broke deadline
  waitfornextperiod();
  wait(1666);
  if(deadline())
    if(show) printf("Deadline (+666, bye bye...):        %10d ms \n", deadline());
  // test result ok?
  if (elapsed2 == 123)
    res = 1;
  if(show) printf("\ntimer_test result: %s\n", res ? "true" : "false");
  return res;
}

int checksum_test(){
  int res = 0;
  char mydata[] = {0x01, 0x02, 0x03, 0x04};
  //printf("elems in mydata=%lu\n",sizeof(mydata)/sizeof(mydata[0]));
  int chksum = datasum(mydata, sizeof(mydata)/sizeof(mydata[0]));
  if (chksum == 0x0406) res = 1;
  else res = 0;
  //printf("Checksum: 0x%X\n", chksum);
  char mydata2[] = {0x01, 0x02, 0x03};
  int chksum2 = datasum(mydata2, sizeof(mydata2)/sizeof(mydata2[0]));
  if (chksum == 0x0402) res = 1;
  else res = 0;
  //printf("Checksum2: 0x%X\n", chksum2);
  //checksum_test2(mydata);
  // 0xb861 is the checksum
  // 4500 0073 0000 4000 4011 *b861* c0a8 0001 c0a8 00c7
  char mydata3[] = {0x45, 0x00, 0x00, 0x73, 0x00, 0x00,
                    0x40, 0x00, 0x40, 0x11,  0xb8, 0x61, 0xc0, 0xa8,
                    0x00, 0x01, 0xc0, 0xa8, 0x00, 0xc7};
  int chksum3 = datasum(mydata3, sizeof(mydata3)/sizeof(mydata3[0]));
  //printf("...chksum3=0x%X\n", chksum3);
  chksum3 = checksum(chksum3);
  if (chksum3 == 0x0000) res = 1;
  else res = 0;
  //printf("Checksum3: 0x%X\n", chksum3);
  return res;
}

int mod_test(){
  int res = 0; //false
  if (mod(10,3) == 1)
    res = 1;
  return res;
}

//*****************************************************************************
// MAIN SECTION: Place various "main" functions here and enable as needed
//*****************************************************************************

// Basic testing
int main() {
  printf("Hello tpip world! \n\n");
  // some tests
  if(!checksum_test()) printf("checksum_test() failed\n");
  if(!timer_test()) printf("timer_test() failed\n");
  if(!mod_test()) printf("mod_test() failed\n");
  return 0;
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