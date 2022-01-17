/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl (martin@jopdesign.com), Rasmus Ulslev Pedersen
  License: Simplified BSD
*/


#include <stdio.h>
#include <time.h>
#include "eth_patmos_io.h"
#include "mac.h"
#include "eth_mac_driver.h"
#include "tpip.h"

unsigned char my_ip[4] = {192, 168, 24, 50};

void printipaddr(unsigned long ipaddr, char* ipstr)
{
  sprintf(ipstr, "%d.%d.%d.%d", (int)((ipaddr >> 24) & 0xFF), (int)((ipaddr >> 16) & 0xFF), 
                          (int)((ipaddr >> 8) & 0xFF), (int)(ipaddr & 0xFF));
}
void tpip_print_ip(unsigned int ip){
  char ipstr[20]; 
  printipaddr(ip, ipstr);
  printf("IP Address       = 0x%08x (%s)\n", ip, ipstr);
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

int tpip_compare_ip(unsigned char * my_ip, unsigned int destination_ip){
  for(int i = 0; i < 4; i++){
    if(my_ip[i] != ((destination_ip >> (24-8*i)) & 0xFF)){
      printf("my_IP[%d] is: %d", i, my_ip[i]);
      printf("destination IP[%d] is: %d\n", i, ((destination_ip >> (24-8*i)) & 0xFF));
      return 0;
    }
      
  }
  return 1;
}
__attribute__((noinline)) 
int packip(unsigned char *netbuf, const ip_t *ip)
{
// issue: writing byte, byte, and unsigned short does not work
// wordaround: write the whole word at the same time
//  *netbuf = ip->verhdl; netbuf += 1;
//  *netbuf = ip->tos; netbuf += 1;
//  *((unsigned short*)netbuf) = htons(ip->length); netbuf += 2;
  printf("++ip->verhdl = 0x%08x\n", ip->verhdl);
  printf("hex word 0 test= 0x%08x\n", ((((unsigned int)ip->verhdl) << 24) | (((unsigned int)ip->tos) << 16) | htons(((unsigned int)ip->length))));
  *((unsigned int*)netbuf) = htonl((((unsigned int)ip->verhdl) << 24) | (((unsigned int)ip->tos) << 16) | htons(((unsigned int)ip->length))); 
  printf("hex word 0 = 0x%08x\n", *((unsigned int*)netbuf)); netbuf += 4;
  //  *(unsigned short*)netbuf = htons(ip->id);           netbuf += 2;
  //  *(unsigned short*)netbuf = htons(ip->ff);           netbuf += 2;
  *((unsigned int*)netbuf) = htonl((htons(ip->id) << 16) | (htons(ip->ff))); 
  printf("hex word 1 = 0x%08x\n", *((unsigned int*)netbuf)); netbuf += 4;
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
  printf("Buf index: %d \n", *buf);
  // One strange Bug: the first 2 bytes srcip will duplicate checksum, only srcip and dstip get this
  // from srcport is normal
  // ip->srcip = ntohl(*((unsigned int*)(buf))); buf +=4;
  ip->srcip = *buf << 24 | *(++buf) << 16 | *(++buf) << 8 | *(++buf); 
  printf("Source IP: %d \n", ip->srcip);
  // ip->dstip = ntohl(*((unsigned int*)(buf))); buf +=4;
  ip->dstip = *(++buf) << 24 | *(++buf) << 16 | *(++buf) << 8 | *(++buf);
  buf++;
  ip->udp.srcport = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.dstport = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.length = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.checksum = ntohs(*((unsigned short*)(buf))); buf++; buf++;

  for (int i = 0; i < ip->udp.length - 8; buf++, i++)
    ip->udp.data[i] = *buf;
}

//todo: fix bug on printing (unaligned memory access on patmos?)
void bufprint(const unsigned char* pbuf, int cnt)
{
    for(int i = 0; i < cnt; i++)
    {
        if(i == 0)
            printf("0000: ");
        else if(i % 4 == 0)
            printf("\n%04d: ", i);

        printf("0x%02x ", (unsigned int)(*(pbuf + i)));
    }  
    printf("\n");
    for(int i = 0; i < cnt; i++)
    {
        printf("%02d: 0x%02x\n", i, *(pbuf + i));
    }  
    printf("\n");
    for(int i = 0; i < cnt/4; i++)
    {
        printf("word %02d: 0x%08x\n", i, *((unsigned int*)(pbuf + (i*4))));
    }  
    printf("\n");
}

int tpip_get_length(unsigned char* bufin){
  unsigned short length;
  length = bufin[14+2]; // From IPV4 Header
  length = length << 8;
	length = length + bufin[14+3]; 
	return length;
}
int tpip_load_bufin(unsigned int rx_addr, unsigned char* bufin){
  unsigned char dst_mac[6];
  int length;
  enum eth_protocol protocol_type;
  // mac_addr_dest(rx_addr,dst_mac);
  // for(int i = 0; i < 6; i++){
  //   if(dst_mac[i] != my_mac[i]){
  //     printf("destination mac does not match! \n");
  //     return 0;
  //   } 
  // }
  protocol_type = mac_packet_type(rx_addr);
  switch (protocol_type)
  {
    case UNSUPPORTED:
      printf("Unknown protocol, filtered out. \n");
      return 0;

    case UDP:
      printf("A UDP packet is received. \n");
      length = tpip_get_length(bufin);
      for(int i = 0; i < length; i++){
        bufin[i] = mem_iord_byte(rx_addr + i);
      }
    break;
  
    case ARP: // Has a fixed length
      printf("An ARP packet is received. \n");
      for(int i =0; i < 42; i++){
        bufin[i] = mem_iord_byte(rx_addr + i);
      }
    break;

    default:
      break;
  }

  return 1;
}

int tpip_load_bufout(unsigned int tx_addr, unsigned char* bufout, unsigned int length){
  printf("Loading MAC: ");
  for(int i = 0; i < length; i++){
    printf("BUFFER DATA: %d ", bufout[i]);
    mem_iowr_byte(tx_addr + i, bufout[i]);
    printf("MAC buffer data: %d \n", mem_iord_byte(tx_addr + i));
  }
  for(int i = 0; i < length; i++){
    if(mem_iord_byte(tx_addr + i) != bufout[i]){
      return 0;
    }
  }
  return 1;
}

// // CHECKSUM UTILITIES (on the original structs, so that is to be updated)
int tpip_verify_checksum(unsigned char* ip){
  unsigned int checksum;
	checksum = 0;
	_Pragma("loopbound min 0 max 10")
	for (int i = 0; i<20; i=i+2){
		checksum = checksum + (ip[14 + i] << 8) + (ip[15 + i] & 0xFF);
	}
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = ((~checksum) & 0xFFFF);
	if (checksum == 0){
		return 1;
	}else{
		return 0;
	}	
}
int tpip_udp_verify_checksum(ip_t *ip) {
  unsigned short int udp_length;
	unsigned short int corrected_length;
	unsigned int checksum = 0;
	udp_length = ip->udp.length;
  for(int i = 0; i < udp_length -8; i++){
    printf("UDP DATA[%d]: %02x",i, ip->udp.data[i]);
  }
	printf("\n");
	if ((udp_length & 0x1) == 0){
		//even
		corrected_length = udp_length;
	}else{
		//odd, add 8 bits zero at the end of data
		corrected_length = udp_length + 1;
    printf("ip->udp.data[udp_length-8] = %d \n", ip->udp.data[udp_length-8]);
    printf("ip->udp.data[udp_length-9] = %d \n", ip->udp.data[udp_length-1-8]);
    
		ip->udp.data[udp_length-8] = 0x00;
	}
  printf("Corrected_length: %d \n", corrected_length);
  checksum = ip->udp.srcport + ip->udp.dstport + ip->udp.length + ip->udp.checksum;
	printf("temp check sum: %d \n", checksum);
  _Pragma("loopbound min 0 max 14") // why max 28????????
	for (int i = 0; i<corrected_length - 8; i = i+2){
		checksum = checksum + (ip->udp.data[i] << 8) + (ip->udp.data[i+1] & 0xFF);
    printf("checksum udp data: %d\n", ip->udp.data[i]);
	}
	
  // UDP pseudo header: Use the IP src and dest
  checksum = checksum + 
              (ip->srcip >> 16) + (ip->srcip & 0xFFFF) + 
              (ip->dstip >> 16) + (ip->dstip & 0xFFFF);
  // UDP pseudo header: zero | Protocol Number | UDP length
	checksum = checksum + 0x0011 + udp_length;
  printf("1. check sum: %d \n", checksum);
  
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = ((~checksum) & 0xFFFF);
  printf("checksum: %d\n", checksum);
	if (checksum == 0){
		return 1;
	}else{
		return 0;
	}		
}
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

