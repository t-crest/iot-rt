/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl (martin@jopdesign.com), Rasmus Ulslev Pedersen
  License: Simplified BSD
*/


#include <stdio.h>
#include <time.h>
#include "tpip.h"

void printipaddr(unsigned long ipaddr, char* ipstr)
{
  sprintf(ipstr, "%d.%d.%d.%d", (int)((ipaddr >> 24) & 0xFF), (int)((ipaddr >> 16) & 0xFF), 
                          (int)((ipaddr >> 8) & 0xFF), (int)(ipaddr & 0xFF));
}

void printipdatagram(ip_t *ip){
  printf("ip->verhdl       = 0x%02x\n", ip->verhdl);
  printf("ip->tos          = 0x%02x\n", ip->tos);
  printf("ip->length       = 0x%04x (%d)\n", ip->length, ip->length);
  printf("ip->id           = 0x%04x\n", ip->id);
  printf("ip->ff           = 0x%04x\n", ip->ff);
  printf("ip->ttl          = 0x%02x\n", ip->ttl);
  printf("ip->prot         = 0x%02x\n", ip->prot);
  printf("ip->checksum     = 0x%04x\n", ip->checksum);
  char ipstr[20]; 
  printipaddr(ip->srcip, ipstr);
  printf("ip->srcip        = 0x%08x (%s)\n", (unsigned int)ip->srcip, ipstr);
  printipaddr(ip->dstip, ipstr);
  printf("ip->dstip        = 0x%08x (%s)\n", (unsigned int)ip->dstip, ipstr);
  printf("ip->udp.srcport  = 0x%04x\n", ip->udp.srcport); 
  printf("ip->udp.dstport  = 0x%04x\n", ip->udp.dstport);
  printf("ip->udp.length   = 0x%04x\n", ip->udp.length);
  printf("ip->udp.checksum = 0x%04x\n", ip->udp.checksum); 
  for (int i = 0; i < ip->udp.length-8; i++)
    printf("ip->udp.data[%02d] = 0x%02x\n", i, ip->udp.data[i]);
}

__attribute__((noinline)) 
int packip(unsigned char *netbuf, const ip_t *ip)
{
// issue: writing byte, byte, and unsigned short does not work
// wordaround: write the whole word at the same time
//  *netbuf = ip->verhdl; netbuf += 1;
//  *netbuf = ip->tos; netbuf += 1;
//  *((unsigned short*)netbuf) = htons(ip->length); netbuf += 2;
  *((unsigned int*)netbuf) = htonl((ip->verhdl << 24) | (ip->tos << 16) | htons(ip->length)); netbuf += 4;
//  *(unsigned short*)netbuf = htons(ip->id);           netbuf += 2;
//  *(unsigned short*)netbuf = htons(ip->ff);           netbuf += 2;
  *((unsigned int*)netbuf) = htonl((htons(ip->id) << 16) | (htons(ip->ff))); netbuf += 4;
  //*netbuf = ip->ttl;                                  netbuf += 1;
  //*netbuf = ip->prot;                                 netbuf += 1;
  //*(unsigned short*)netbuf = htons(ip->checksum);     netbuf += 2;
  *((unsigned int*)netbuf) = (ip->ttl << 24) | (ip->prot << 16) | htons(ip->checksum); netbuf += 4;
  *(unsigned int*)netbuf = htonl(ip->srcip);          netbuf += 4;
  *(unsigned int*)netbuf = htonl(ip->dstip);          netbuf += 4;
  *(unsigned short*)netbuf = htons(ip->udp.srcport);  netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->udp.dstport);  netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->udp.length);   netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->udp.checksum); netbuf += 2;

  int udpcnt = ip->udp.length - 8;
  _Pragma("loopbound min 0 max 8")
  for (int i = 0; i < udpcnt; i++, netbuf++)
    *netbuf = ip->udp.data[i]; 

  return ip->length;
}

void unpackip(ip_t *ip, const unsigned char *buf)
{
  ip->verhdl = *buf; buf++;
  ip->tos = *buf; buf++;
  ip->length = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->id = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->ff = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->ttl = *buf; buf++;
  ip->prot = *buf; buf++;
  ip->checksum = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->srcip = ntohl(*((unsigned long*)(buf))); buf +=4;
  ip->dstip = ntohl(*((unsigned long*)(buf))); buf +=4;

  ip->udp.srcport = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.dstport = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.length = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.checksum = ntohs(*((unsigned short*)(buf))); buf++; buf++;

  for (int i = 0; i < ip->udp.length - 8; buf++, i++)
    ip->udp.data[i] = *buf;
}

void bufprint(const unsigned char* pbuf, int cnt)
{
    for(int i = 0; i < cnt; i++)
    {
        if(i == 0)
            printf("0000: ");
        else if(i % 4 == 0)
            printf("\n%04d: ", i);

        printf("0x%02x ", *(pbuf + i));
    }  
printf("\n");
    for(int i = 0; i < cnt; i++)
    {
        printf("%02d: 0x%02x\n", i, *(pbuf + i));
    }  
    printf("\n");
}

// // CHECKSUM UTILITIES (on the original structs, so that is to be updated)

void ipchecksum(unsigned char* ipraw){
  unsigned int checksum = 0;
  checksum += (ipraw[ 0] << 8) | ipraw[ 1]; // version, ihl and tos
  checksum += (ipraw[ 2] << 8) | ipraw[ 3]; // total length
  checksum += (ipraw[ 4] << 8) | ipraw[ 5]; // identification 
  checksum += (ipraw[ 6] << 8) | ipraw[ 7]; // flags and fragment offset
  checksum += (ipraw[ 8] << 8) | ipraw[ 9]; // ttl and protocol
  checksum += (ipraw[12] << 8) | ipraw[13]; // src IP 
  checksum += (ipraw[14] << 8) | ipraw[15]; // src IP 
  checksum += (ipraw[16] << 8) | ipraw[17]; // src IP 
  checksum += (ipraw[18] << 8) | ipraw[19]; // src IP 
  checksum  = (checksum & 0xFFFF) + (checksum >> 16);
  ipraw[10] = checksum >> 8;
  ipraw[11] = checksum & 0xFF;
}


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

