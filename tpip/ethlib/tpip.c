/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl (martin@jopdesign.com), Rasmus Ulslev Pedersen
  License: Simplified BSD
*/
#include "tpip.h"

unsigned char my_ip[4] = {192, 168, 24, 50};

void printipaddr(unsigned long ipaddr, char* ipstr)
{
  sprintf(ipstr, "%d.%d.%d.%d", (int)((ipaddr >> 24) & 0xFF), (int)((ipaddr >> 16) & 0xFF), 
                          (int)((ipaddr >> 8) & 0xFF), (int)(ipaddr & 0xFF));
}
void tpip_print_ip(unsigned int ip){
  char ipstr[4]; 
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

__attribute__((noinline)) 
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
int packip(unsigned char *netbuf, ip_t *ip)
{
// issue: writing byte, byte, and unsigned short does not work
// wordaround: write the whole word at the same time
// printf("ip-t address: %p \n", ip);

// printf("ip-t verhdl address bef: %p\n", &ip->verhdl);
ip->verhdl = 0x45;
    ip->tos    = 0x00;
    ip->length = 49;
    ip->id     = 1;
    ip->ff     = 0x4000;
    ip->ttl    = 0x40;
    ip->prot   = 0x11;
    ip->checksum = 0x0000;
    ip->srcip  = (192 << 24) | (168 << 16) | (24 << 8) | 50; // Patmos IP addr
    ip->dstip  = (192 << 24) | (168 << 16) | (24 << 8) | 10; // my linux machine
  // printf("verhdl: 0x%04x, tos: 0x%04x \n", ip->verhdl, ip->tos);
  // printf("length: 0x%04x\n", ip->length);
  *(unsigned short*) netbuf = ip->verhdl << 8 | ip->tos; 
  // printf("ip length: 0x%04x\n ", *((unsigned short*) netbuf)); 
  netbuf += 2;
  *((unsigned short*)netbuf) = htons(ip->length); 
  // printf("ip length: 0x%04x\n ", *((unsigned short*) netbuf)); 
  netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->id);           netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->ff);           netbuf += 2;
  *(unsigned short*)netbuf = ip->ttl << 8 | ip->prot;        netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->checksum);     netbuf += 2;
  //printf("Byte After Checksum: %04x \n", *netbuf); netbuf += 2;
  
  for(int i =0; i < 4; i++){
    *netbuf = (ip->srcip >> (24 - 8*i)) & 0xFF;
    // printf("IP SOURCE: %08d", *netbuf);
    
    netbuf++;
  }
  for(int i =0; i < 4; i++){
    *netbuf = (ip->dstip >> (24 - 8*i)) & 0xFF;
    // printf("IP dest: %08d", *netbuf);
    netbuf++;
  }
    
  *(unsigned short*)netbuf = htons(ip->udp.srcport);  netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->udp.dstport);  netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->udp.length);   netbuf += 2;
  *(unsigned short*)netbuf = htons(ip->udp.checksum); netbuf += 2;
  int udpcnt = ip->udp.length - 8;
   _Pragma("loopbound min 0 max 50")
  for(int i = 0; i < udpcnt; i++, netbuf++){
    // printf("ip->DATA[%d] = %c   ", i, ip->udp.data[i]);
    *netbuf = ip->udp.data[i];
    // printf("DATA[%d] = %c", i, *netbuf);
  }

  return ip->length;
}

__attribute__((noinline)) 
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
  //printf("Buf index: %d \n", *buf);
  // One strange Bug: the first 2 bytes srcip will duplicate checksum, only srcip and dstip get this
  // from srcport is normal
  // ip->srcip = ntohl(*((unsigned int*)(buf))); buf +=4;
  ip->srcip = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; buf += 4; 
  //printf("Source IP: %d \n", ip->srcip);
  // ip->dstip = ntohl(*((unsigned int*)(buf))); buf +=4;
  ip->dstip = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; buf += 4;
  //buf++;
  ip->udp.srcport = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.dstport = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.length = ntohs(*((unsigned short*)(buf))); buf++; buf++;
  ip->udp.checksum = ntohs(*((unsigned short*)(buf))); buf++; buf++;

  #pragma loopbound min 0 max 50
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

__attribute__((noinline)) 
int tpip_get_length(unsigned char* bufin){
  unsigned short length;
  length = bufin[14+2]; // From IPV4 Header
  length = length << 8;
	length = length + bufin[14+3]; 
	return length;
}

__attribute__((noinline)) 
int tpip_load_bufin(unsigned int rx_addr, unsigned char* bufin){
  unsigned char dst_mac[6];
  int length;
  enum eth_protocol protocol_type;
  
  protocol_type = mac_packet_type(rx_addr);
  if(protocol_type == UNSUPPORTED){
    printf("Unknown protocol, filtered out. \n");
    return 0;
    
  }
  else if(protocol_type == ICMP){
    printf("ICMP packet is received. \n");
    length = tpip_get_length(bufin);
    #pragma loopbound min 0 max 98
    for(int i = 0; i < length+14; i++){
      bufin[i] = mem_iord_byte(rx_addr + i);
    }
    
  }
  else if(protocol_type == UDP){
    printf("A UDP packet is received. \n");
    length = tpip_get_length(bufin);
    #pragma loopbound min 0 max 100
    for(int i = 0; i < length+14; i++){
      bufin[i] = mem_iord_byte(rx_addr + i);
    }
  }
  else if(protocol_type == ARP){
    printf("An ARP packet is received. \n");
    for(int i =0; i < 42; i++){
      bufin[i] = mem_iord_byte(rx_addr + i);
    } 
  }

  return 1;
}

__attribute__((noinline)) 
int tpip_load_bufout(unsigned int tx_addr, unsigned char* bufout, unsigned int length){
  // printf("Loading MAC: ");
  // printf("length is %d \n", length);
  #pragma loopbound min 0 max 100
  for(int i = 0; i < length; i++){
    //printf("BUFFER DATA: %04x ||||||  ", bufout[i]);
    mem_iowr_byte(tx_addr + i, bufout[i]);
    //printf("MAC buffer data: %04x \n", mem_iord_byte(tx_addr + i));
  }
  #pragma loopbound min 0 max 100
  for(int i = 0; i < length; i++){
    if(mem_iord_byte(tx_addr + i) != bufout[i]){
      return 0;
    }
  }
  return 1;
}

// // CHECKSUM UTILITIES (on the original structs, so that is to be updated)
__attribute__((noinline)) 
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
__attribute__((noinline)) 
int tpip_udp_verify_checksum(ip_t *ip) {
  unsigned short int udp_length;
	unsigned short int corrected_length;
	unsigned int checksum = 0;
	udp_length = ip->udp.length;
  // for(int i = 0; i < udp_length -8; i++){
  //   printf("UDP DATA[%d]: %02x",i, ip->udp.data[i]);
  // }
	// printf("\n");
	if ((udp_length & 0x1) == 0){
		//even
		corrected_length = udp_length;
	}else{
		//odd, add 8 bits zero at the end of data
		corrected_length = udp_length + 1;
    // printf("ip->udp.data[udp_length-8] = %d \n", ip->udp.data[udp_length-8]);
    // printf("ip->udp.data[udp_length-9] = %d \n", ip->udp.data[udp_length-1-8]);
    
		ip->udp.data[udp_length-8] = 0x00;
	}
  // printf("Corrected_length: %d \n", corrected_length);
  checksum = ip->udp.srcport + ip->udp.dstport + ip->udp.length + ip->udp.checksum;
	// printf("temp check sum: %d \n", checksum);
  _Pragma("loopbound min 0 max 14") // why max 28????????
	for (int i = 0; i<corrected_length - 8; i = i+2){
		checksum = checksum + (ip->udp.data[i] << 8) + (ip->udp.data[i+1] & 0xFF);
    // printf("checksum udp data: %d\n", ip->udp.data[i]);
	}
	
  // UDP pseudo header: Use the IP src and dest
  checksum = checksum + 
              (ip->srcip >> 16) + (ip->srcip & 0xFFFF) + 
              (ip->dstip >> 16) + (ip->dstip & 0xFFFF);
  // UDP pseudo header: zero | Protocol Number | UDP length
	checksum = checksum + 0x0011 + udp_length;
  // printf("1. check sum: %d \n", checksum);
  
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = ((~checksum) & 0xFFFF);
  // printf("checksum: %d\n", checksum);
	if (checksum == 0){
		return 1;
	}else{
		return 0;
	}		
}
__attribute__((noinline)) 
void udpchecksum(unsigned char* udpraw){
  unsigned int checksum = 0;
  // UDP pseudo header: Src IP | Dst IP | 0X00 | protocol 0x11(UDP) | UDP LENGTH
  checksum += (udpraw[12] << 8) | udpraw[13]; // src IP 
  checksum += (udpraw[14] << 8) | udpraw[15]; // src IP 
  checksum += (udpraw[16] << 8) | udpraw[17]; // dst IP 
  checksum += (udpraw[18] << 8) | udpraw[19]; // dst IP 
  checksum += (0x00 << 8) | udpraw[ 9]; // 0x00 and protocol
  checksum += (udpraw[24] << 8) | udpraw[25]; // UDP length
  // UDP header 
  checksum += (udpraw[20] << 8) | udpraw[21]; // UDP src port
  checksum += (udpraw[22] << 8) | udpraw[23]; // UDP dst port
  checksum += (udpraw[24] << 8) | udpraw[25]; // UDP length
  // checksum += (udpraw[26] << 8) | udpraw[27]; // UDP checksum
  // UDP data
  int udp_length = (udpraw[24] << 8) | udpraw[25];
  if ((udp_length & 0x1) == 1){
		//odd, add 8 bits zero at the end of data
		udp_length = udp_length + 1;
		udpraw[udp_length + 20] = 0x00;
	}
  #pragma loopbound min 0 max 50
  for(int i = 28; i < udp_length; i = i+2){
    checksum += (udpraw[i] << 8) | udpraw[i+1];
  }
  
  if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	if ((checksum & 0xFFFF0000) > 0)
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
  udpraw[26] = checksum >> 8;
  udpraw[27] = checksum & 0xFF;
}
__attribute__((noinline)) 
void ipchecksum(unsigned char* ipraw){
  unsigned int checksum = 0;
  checksum += (ipraw[ 0] << 8) | ipraw[ 1]; // version, ihl and tos
  checksum += (ipraw[ 2] << 8) | ipraw[ 3]; // total length
  checksum += (ipraw[ 4] << 8) | ipraw[ 5]; // identification 
  checksum += (ipraw[ 6] << 8) | ipraw[ 7]; // flags and fragment offset
  checksum += (ipraw[ 8] << 8) | ipraw[ 9]; // ttl and protocol
  checksum += (ipraw[12] << 8) | ipraw[13]; // src IP 
  checksum += (ipraw[14] << 8) | ipraw[15]; // src IP 
  checksum += (ipraw[16] << 8) | ipraw[17]; // dst IP 
  checksum += (ipraw[18] << 8) | ipraw[19]; // dst IP 
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
// int calculateipchecksum(ipstruct_t *ip_p) __attribute__((noinline));
// int calculateipchecksum(ipstruct_t *ip_p)
// {
//   unsigned char *h = ip_p->header;
//   int checksum = ((h[0] << 8) + h[1]) + ((h[2] << 8) + h[3]) +
//                  ((h[4] << 8) + h[5]) + ((h[6] << 8) + h[7]) +
//                  ((h[8] << 8) + h[9]) + // ignore old checksum: (h[10]<<8)+h[11]
//                  ((h[12] << 8) + h[13]) + ((h[14] << 8) + h[15]) +
//                  ((h[16] << 8) + h[17]) + ((h[18] << 8) + h[19]);
//   if ((checksum & 0xFFFF0000) > 0)
//     checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
//   if ((checksum & 0xFFFF0000) > 0)
//     checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
//   if ((checksum & 0xFFFF0000) > 0)
//     checksum = (checksum >> 16) + (checksum & 0x0000FFFF);
//   checksum = (~checksum) & 0x0000FFFF;
//   return checksum;
// }

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

