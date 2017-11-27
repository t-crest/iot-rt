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
static int buf_bytes[MAX_BUF_NUM];

//*****************************************************************************
// FORWARD DECLARATIONS (only on a "need-to" basis)
//*********************************************************************


//*****************************************************************************
// UTILITIES SECTION
//*****************************************************************************

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

//*****************************************************************************
// UDP SECTION
//*****************************************************************************

//*****************************************************************************
// IP SECTION
//*****************************************************************************

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


//*****************************************************************************
// MAIN SECTION: Place various "main" functions here and enable as needed
//*****************************************************************************

// Basic testing
int main() {
  printf("Hello tpip world! \n\n");
  
  timer_test();

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
