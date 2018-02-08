/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl (martin@jopdesign.com), Rasmus Ulslev Pedersen
  License: Simplified BSD
*/

// TODOs
// 0. use a struct to work with the packet and not "raw" memory DONE
// 1. one UDP packet over slip
// 2. UDP "reply" over slip

#include <stdio.h>
#include "tpip.h"

void printip(unsigned long ip)
{
  printf("%d.%d.%d.%d\n", (int)((ip >> 24) & 0xFF), (int)((ip >> 16) & 0xFF), (int)((ip >> 8) & 0xFF), (int)(ip & 0xFF));
}

__attribute__((noinline)) int packip(unsigned long networkbuf[], const ip_t *ip)
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

void unpackip(ip_t *ip, const unsigned int networkbuf[])
{
  unsigned int word = ntohl(networkbuf[1]);
  ip->verhdl = (unsigned char)(word >> 24);
  ip->tos = (unsigned char)((word >> 16) & 0xFF);
  ip->length = ntohs(word & 0xFFFF);
  word = ntohl(networkbuf[2]);
  ip->id = ntohs((word >> 16) & 0xFFFF);
  ip->ff = ntohs(word & 0xFFFF);
  word = ntohl(networkbuf[3]);
  ip->ttl = (unsigned char)(word >> 24);
  ip->prot = (unsigned char)((word >> 16) & 0xFF);
  ip->checksum = ntohs(word & 0xFFFF);
  ip->srcip = ntohl(networkbuf[4]);
  ip->dstip = ntohl(networkbuf[5]);

  word = ntohl(networkbuf[6]);
  ip->udp.srcport = ntohs((word >> 16) & 0xFFFF);
  ip->udp.dstport = ntohs(word & 0xFFFF);

  word = ntohl(networkbuf[7]);
  ip->udp.length = ntohs((word >> 16) & 0xFFFF);
  ip->udp.checksum = ntohs(word & 0xFFFF);

  int udpdatawords = (ip->udp.length / 4) - 2;
  for (int i = 0; i < udpdatawords; i++)
  {
    word = ntohl(networkbuf[8 + i]);
    ip->udp.data[i * 4] = (unsigned char)(word >> 24);
    ip->udp.data[i * 4 + 1] = (unsigned char)((word >> 16) & 0xFF);
    ip->udp.data[i * 4 + 2] = (unsigned char)((word >> 8) & 0xFF);
    ip->udp.data[i * 4 + 3] = (unsigned char)(word & 0xFF);
  }

  // check
  //if ((1 + 5 + 2 + udpdatawords) != ntohl(networkbuf[0]))
    //printf("error: networkbuf[0]=0x%08x\n", (unsigned int)ntohl(networkbuf[0]));
}