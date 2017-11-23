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
#define PERIOD 1000

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
static int buf_bytes[MAX_BUF_NUM];

//*****************************************************************************
// FORWARD DECLARATIONS (on "need-to" basis)
//*********************************************************************

//void sizeinfo();

//*****************************************************************************
// UTILITIES SECTION
//*****************************************************************************

// function for getting number of milliseconds since epoc (1970)
// used with waitfornextperiod
long long int currenttimemillis(){
  struct timeval timenow; 
  gettimeofday(&timenow, NULL);    
  long long int msec = ((long long int) timenow.tv_sec) * 1000LL + (long long int) timenow.tv_usec / 1000LL;
  return msec;
}

// will block (by spinning) until next period
void waitfornextperiod(){
  long long int now = currenttimemillis();
  while(currenttimemillis() - now < PERIOD);
}

//*****************************************************************************
// UDP SECTION
//*****************************************************************************

//*****************************************************************************
// IP SECTION
//*****************************************************************************

//*****************************************************************************
// MAIN SECTION: Place various "main" functions here and enable as needed
//*****************************************************************************

// Basic testing
int main() {
  printf("Hello tpip world!\n");
  
  printf("Timer before 'waitforoneperiod()': %lld\n", currenttimemillis());
  waitfornextperiod();
  printf("Timer after 'waitforoneperiod()': %lld\n", currenttimemillis());

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
