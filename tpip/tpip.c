/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl (martin@jopdesign.com), Rasmus Ulslev Pedersen
  License: Simplified BSD
*/


#include <stdio.h>
#include "tpip.h"

void printipaddr(unsigned long ipaddr)
{
  printf("%d.%d.%d.%d\n", (int)((ipaddr >> 24) & 0xFF), (int)((ipaddr >> 16) & 0xFF), 
                          (int)((ipaddr >> 8) & 0xFF), (int)(ipaddr & 0xFF));
}

void printipdatagram(ip_t *ip){
  printf("ip->verhdl = 0x%02x\n", ip->verhdl);
  printf("ip->tos = 0x%02x\n", ip->tos);
  printf("ip->length = 0x%04x %d\n", ip->length, ip->length);
  printf("ip->id = 0x%04x\n", ip->id);
  printf("ip->ff = 0x%04x\n", ip->ff);
  printf("ip->ttl = 0x%02x\n", ip->ttl);
  printf("ip->prot = 0x%02x\n", ip->prot);
  printf("ip->checksum = 0x%04x\n", ip->checksum);
  printf("ip->srcip = 0x%08x ", (unsigned int)ip->srcip); printipaddr(ip->srcip);
  printf("ip->dstip = 0x%08x ", (unsigned int)ip->dstip); printipaddr(ip->dstip);
  printf("ip->udp.srcport = 0x%04x\n", ip->udp.srcport); 
  printf("ip->udp.dstport = 0x%04x\n", ip->udp.dstport);
  printf("ip->udp.length = 0x%04x\n", ip->udp.length);
  printf("ip->udp.checksum = 0x%08x\n", ip->udp.checksum); 
  for (int i = 0; i < ip->udp.length && i < 10; i++)
  {
    printf("ip->udp.data[%02d] = 0x%02x\n", i, ip->udp.data[i]);
  }
}

__attribute__((noinline)) 
int packip(unsigned int networkbuf[], const ip_t *ip)
{
  networkbuf[1] = htonl((ip->verhdl << 24) | (ip->tos << 16) | htons(ip->length));
  networkbuf[2] = htonl((htons(ip->id) << 16) | htons(ip->ff));
  networkbuf[3] = htonl((ip->ttl << 24) | (ip->prot << 16) | htons(ip->checksum));
  networkbuf[4] = htonl(ip->srcip);
  networkbuf[5] = htonl(ip->dstip);
  networkbuf[6] = htonl((htons(ip->udp.srcport) << 16) | htons(ip->udp.dstport));
  networkbuf[7] = htonl((htons(ip->udp.length) << 16) | htons(ip->udp.checksum));

  int udpdatawords = (ip->udp.length / 4) - 2;

  _Pragma("loopbound min 0 max 8")
  for (int i = 0; i < udpdatawords; i++)
     networkbuf[8 + i] = htonl(ip->udp.data[i * 4] << 24 |
                               ip->udp.data[i * 4 + 1] << 16 |
                               ip->udp.data[i * 4 + 2] << 8 |
                               ip->udp.data[i * 4 + 3]);

  networkbuf[0] = htonl(1 + 5 + 2 + udpdatawords);
  return networkbuf[0];
}

void unpackip(ip_t *ip, const unsigned char *buf)
{
  printf("buf:\n");
  bufprint(buf, 32);
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

void bufprint(const char* pbuf, int cnt)
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
}
