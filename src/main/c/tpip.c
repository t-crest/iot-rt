/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl, Rasmus Ulslev Pedersen
  License: Simplified BSD
*/

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

// number of bytes in int
#define INTBYTES = 4

//sizeof(arr)/sizeof(arr[0]);

//*****************************************************************************
// FORWARD DECLARATIONS (only on a "need-to" basis)
//*********************************************************************


//*****************************************************************************
// UTILITIES SECTION
//*****************************************************************************

// TIMER UTILITIES

// function for getting number of milliseconds since "system-epoc"
// used with waitfornextperiod
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
void wait(int waittime){
  int now = currenttimemillis();
  while(currenttimemillis()-now < waittime);
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
printf("arraysize=%d\n", arraysize);
  for(int i = 0; i < arraysize; i = i + 1){    
  printf("...data[%d]=0x%X\n", i, data[i]);
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
printf("datachecksum=0x%X\n", datachecksum);
        }
        if (arraysize % 2 != 0)  // one byte left
            datachecksum = datachecksum + ((int) data[arraysize - 1] << 8);
    }
    return datachecksum;
}

// adds in the potntial carries and finally inverts
int checksum(int chksumcarry) {
  int checksum = chksumcarry;
  if (checksum & 0xFFFF0000 > 0) 
      checksum = (checksum > 16) + (checksum & 0x0000FFFF);
  if (checksum & 0xFFFF0000 > 0)
      checksum = (checksum > 16) + (checksum & 0x0000FFFF);
  if (checksum & 0xFFFF0000 > 0)
      checksum = (checksum > 16) + (checksum & 0x0000FFFF);
  checksum = ~checksum;
  return checksum & 0x0000FFFF;
}


//*****************************************************************************
// UDP SECTION
//*****************************************************************************

//todo: create getter/setter functions for fields
//todo: checksum


//*****************************************************************************
// IP SECTION
//*****************************************************************************

//todo: create getter/setter functions for fields
//todo:checksum

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
  printf("timer_test!\n");
  printf("PERIOD: %d ms \n\n", PERIOD);
  
  //start timer
  initwaitfornextperiod();
  int before = currenttimemillis();
  printf("Timer before 'waitforoneperiod()':  %10d ms \n", before);
  int elapsed1 = waitfornextperiod();
  int after = currenttimemillis();
  printf("Timer after 'waitforoneperiod()':   %10d ms \n", after);
  printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed1);
  printf("Elapsed time since this wfnp call:  %10d ms \n\n", (after-before));
  
  // uncomment 'for'  loop to get a false test
  //for(volatile int i = 0; i < 100000000; i++); // 'volatile' to avoid '-O2' optimization
  wait(123); // 'wait(321)' will give a false test
  printf("Timer before 'waitforoneperiod()':  %10d ms \n", currenttimemillis());
  int elapsed2 = waitfornextperiod();
  printf("Timer after 'waitforoneperiod()':   %10d ms \n", currenttimemillis());
  printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed2);

  //1 ms left
  wait(999);
  printf("Deadline (-1, so hurry up!):        %10d ms \n", deadline());
  
  //broke deadline
  waitfornextperiod();
  wait(1666);
  if(deadline())
    printf("Deadline (+666, bye bye...):        %10d ms \n", deadline());
         
  // test result ok?
  if (elapsed2 == 123)
    res = 1;
  printf("\ntimer_test result: %s\n", res ? "true" : "false"); 
  return res;
}

void checksum_test2(char data[]){
  //data has degraded to a char* so it does not work
  //printf("***elems in mydata=%d\n",(int)ARRAY_SIZE(data));
}

int checksum_test(){
  int res = 0; 
  
  char mydata[] = {0x01, 0x02, 0x03, 0x04};
  printf("elems in mydata=%d\n",sizeof(mydata)/sizeof(mydata[0]));
  int chksum = datasum(mydata, sizeof(mydata)/sizeof(mydata[0]));
  if (chksum == 0x0406) res = 1; 
  else res = 0;
  printf("Checksum: 0x%X\n", chksum);
  char mydata2[] = {0x01, 0x02, 0x03};
  int chksum2 = datasum(mydata2, sizeof(mydata2)/sizeof(mydata2[0]));
  if (chksum == 0x0402) res = 1; 
  else res = 0;
  printf("Checksum2: 0x%X\n", chksum2);
  //checksum_test2(mydata);
  // 0xb861 is the checksum
  // 4500 0073 0000 4000 4011 *b861* c0a8 0001 c0a8 00c7
  char mydata3[] = {0x45, 0x00, 0x00, 0x73, 0x00, 0x00, 
                    0x40, 0x00, 0x40, 0x11,  0xb8, 0x61, 0xc0, 0xa8, 
                    0x00, 0x01, 0xc0, 0xa8, 0x00, 0xc7};
  int chksum3 = datasum(mydata3, sizeof(mydata3)/sizeof(mydata3[0]));
printf("...chksum3=0x%X\n", chksum3);
  chksum3 = checksum(chksum3);
  if (chksum3 == 0x0000) res = 1; 
  else res = 0;
  printf("Checksum3: 0x%X\n", chksum3);
  
  return res;
  
}


//*****************************************************************************
// MAIN SECTION: Place various "main" functions here and enable as needed
//*****************************************************************************

// Basic testing
int main() {
  printf("Hello tpip world! \n\n");
  if(!checksum_test()) printf("checksum_test() failed\n");
  if(!timer_test()) printf("timer_test() failed\n");

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