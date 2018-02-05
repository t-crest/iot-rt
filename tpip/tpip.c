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
#include <arpa/inet.h>
#include "tpip.h"

void printip(unsigned long ip)
{
  printf("%d.%d.%d.%d\n", (int)((ip >> 24) & 0xFF), (int)((ip >> 16) & 0xFF), (int)((ip >> 8) & 0xFF), (int)(ip & 0xFF));
}

void packip(unsigned long networkbuf[], const ip_t *ip)
{
  networkbuf[1] = htonl((ip->verhdl << 24) | (ip->tos << 16) | htons(ip->length));
  networkbuf[2] = htonl((htons(ip->id) << 16) | htons(ip->ff));
  networkbuf[3] = htonl((ip->ttl << 24) | (ip->prot << 16) | htons(ip->checksum));
  networkbuf[4] = htonl(ip->srcip);
  networkbuf[5] = htonl(ip->dstip);
  networkbuf[6] = htonl((htons(ip->udp.srcport) << 16) | htons(ip->udp.dstport));
  networkbuf[7] = htonl((htons(ip->udp.length) << 16) | htons(ip->udp.checksum));

  int udpdatawords = (ip->udp.length / 4) - 2;

  for (int i = 0; i < udpdatawords; i++)
    networkbuf[8 + i] = htonl(ip->udp.data[i * 4] << 24 |
                              ip->udp.data[i * 4 + 1] << 16 |
                              ip->udp.data[i * 4 + 2] << 8 |
                              ip->udp.data[i * 4 + 3]);

  networkbuf[0] = htonl(5 + 2 + udpdatawords);
}

void unpackip(ip_t *ip, const unsigned long networkbuf[])
{
  unsigned long word = ntohl(networkbuf[1]);
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
  if ((udpdatawords + 5 + 2) != ntohl(networkbuf[0]))
    printf("error: networkbuf[0]=%lu\n", (unsigned long)ntohl(networkbuf[0]));
}

// OLD STUFF: can be deleted

//*****************************************************************************
// MAIN SECTION: Place various "main" functions here and enable as needed
//*****************************************************************************

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

//2. Char for the networkbuffer is kind of ok. But word access is more efficient (Do er care?). Anyway, in C one can cast anything to anything. It is just memory. But the words must be aligned on word boundary for Patmos.

//3. There are no connections between your array of IP structs and your networkbuffers.

//4. waitfornextperiod contains a classic mistake as it waits for some time and does not increments a variable with the period. See as a solution: https://github.com/t-crest/iot-rt/blob/master/src/main/scala/rtapi/RtThread.scala Essentioal is line 37

//5. checksum: you could also pad the networkbuffer with missing zeros and have a single loop. When done unconditinally then the networkbuffer needs to be a little bit longer (3 bytes). [...]

//6. UDP field code: with a struct access to the indicidual fields is easier. Maybe not even needing functions for this.

//7. Same for IP.

// // CHECKSUM UTILITIES

// // 16-bit (pairs of two byte array entries)
// int datasum(char data[], int arraysize)
// {
//   //printf("arraysize=%d\n", arraysize);
//   for (int i = 0; i < arraysize; i = i + 1)
//   {
//     //printf("...data[%d]=0x%X\n", i, data[i]);
//   }
//   int datachecksum = 0;
//   if (arraysize == 0)
//   {
//     // nothing to do
//   }
//   else if (arraysize == 1)
//   {
//     datachecksum = ((int)data[0] << 8); // Pad with a "zero"
//   }
//   else
//   {
//     for (int i = 0; i < arraysize - 1; i = i + 2)
//     { // byte "pairs"
//       datachecksum = datachecksum + (((int)data[i] << 8) | ((int)data[i + 1]));
//       //printf("datachecksum=0x%X\n", datachecksum);
//     }
//     if (arraysize % 2 != 0) // one byte left
//       datachecksum = datachecksum + ((int)data[arraysize - 1] << 8);
//   }

//   return datachecksum;
// }

// // ip header checksum calculation
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

// // Martin thinks those different pointers are not the best way
// // to represent IP packets.
// //  RUP->MS: Agree. Would you make a typedef for udp as well? So we can get
// //   both to work at the same time.

// // new proposal for using types (MS)
// // we cannot safely use shorter fields due to
// // possible different byte orders.

// typedef struct ip_t
// {
//   unsigned long length;
//   unsigned long id;
//   unsigned long prot;
//   unsigned long source;
//   unsigned long destination;
// } ip_t;

// // And we need to take care of byteorder (with hton() and ntoh()).

// // use as a start 32-bit arrays to ensure it is word aligned.
// // shall use the MAX_* when agreed on.
// static unsigned long packets[3][1600 / 4];

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

// void setipver(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[0] = (char)((value << 4) | (ip_p->header[0] & HDR_MASK));
// }

// // Header 4-bits .[0H], number of 32-bit words
// int getiphdr(ipstruct_t *ip_p)
// {
//   return ip_p->header[0] & HDR_MASK;
// }

// void setiphdr(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[0] = (char)(ip_p->header[0] & VER_MASK) | (value & HDR_MASK);
// }

// // Type of service 8-bits .[1]
// int getiptos(ipstruct_t *ip_p)
// {
//   return ip_p->header[1];
// }

// void setiptos(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[1] = (unsigned char)value;
// }

// // Total length in byte, 16-bits .[2] .[3]
// int getiptlen(ipstruct_t *ip_p)
// {
//   return (ip_p->header[2] << 8) | ip_p->header[3];
// }

// void setiptlen(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[2] = (unsigned char)(value >> 8);
//   ip_p->header[3] = (unsigned char)value;
// }

// // Identification, 16-bits, .[4] .[5]
// int getipid(ipstruct_t *ip_p)
// {
//   return (ip_p->header[4] << 8) | ip_p->header[5];
// }

// void setipid(ipstruct_t *ip_p, int value)
// {
//   // check >>> or >>
//   ip_p->header[4] = (unsigned char)value >> 8;
//   ip_p->header[5] = (unsigned char)value;
// }

// // Flags, 3-bits, .[6H]
// int getflags(ipstruct_t *ip_p)
// {
//   return ip_p->header[6] >> 5;
// }

// void setipflags(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[6] = (unsigned char)((value & FLAGS_MASK) << 5) | (ip_p->header[6] & 0x1F); //0001_1111
// }

// // Fragment offset, 13-bits, .[6L] .[7]
// int getipfoff(ipstruct_t *ip_p)
// {
//   return ((ip_p->header[6] & 0x1F) << 8) | ip_p->header[7]; //0001_1111
// }

// void setipfoff(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[6] = (unsigned char)(ip_p->header[6] & 0xE0) | (value >> 8);
//   ip_p->header[7] = (unsigned char)value;
// }

// // Time to live, 8-bits, .[8]
// int getipttl(ipstruct_t *ip_p)
// {
//   return ip_p->header[8];
// }

// void setipttl(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[8] = (unsigned char)value;
// }

// // Protocol, 8-bits, .[9]
// int getipprot(ipstruct_t *ip_p)
// {
//   return ip_p->header[9];
// }

// void setipprot(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[9] = (unsigned char)value;
// }

// // Checksum, 16-bits, .[10] .[11]
// int getiphchksum(ipstruct_t *ip_p)
// {
//   return (ip_p->header[10] << 8) | ip_p->header[11];
// }

// void setiphchksum(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[10] = (unsigned char)(value >> 8);
//   ip_p->header[11] = (unsigned char)value;
// }

// // Source IP, 32-bits, .[12] .[13] .[14] .[15]
// int getipsrcip(ipstruct_t *ip_p)
// {
//   return (ip_p->header[12] << 24) | (ip_p->header[13] << 16) |
//          (ip_p->header[14] << 8) | (ip_p->header[15]);
// }

// void setipsrcip(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[12] = (unsigned char)(value >> 24); // check the shift
//   ip_p->header[13] = (unsigned char)(value >> 16);
//   ip_p->header[14] = (unsigned char)(value >> 8);
//   ip_p->header[15] = (unsigned char)value;
// }

// // Destination IP, 32-bits, .[16] .[17] .[18] .[19]
// //int getipdstip(ipstruct_t* ip_p) __attribute__((noinline));
// __attribute__((noinline)) int getipdstip(ipstruct_t *ip_p)
// {
//   return (ip_p->header[16] << 24) | (ip_p->header[17] << 16) |
//          (ip_p->header[18] << 8) | ip_p->header[19];
// }

// void setipdstip(ipstruct_t *ip_p, int value)
// {
//   ip_p->header[16] = (unsigned char)(value >> 24);
//   ip_p->header[17] = (unsigned char)(value >> 16);
//   ip_p->header[18] = (unsigned char)(value >> 8);
//   ip_p->header[19] = (unsigned char)value;
// }

// // payload data (such as ICMP, UDP, or TCP message)
// unsigned char *getipdata(ipstruct_t *ip_p)
// {
//   return ip_p->data;
// }

// void setipdata(ipstruct_t *ip_p, unsigned char *data)
// {
//   ip_p->data = data;
// }

// // ip functions //

// // init ip packet fields
// void initip(ipstruct_t *ip_p,
//             int ver,
//             int hdrwrds,
//             int tos,
//             int id,
//             int flags,
//             int foff,
//             int ttl,
//             int prot,
//             int srcip,
//             int dstip,
//             //int* data,
//             int tlen,
//             int hchksum) // to be calculated:0xFFFF
// {
//   setipver(ip_p, ver);
//   setiphdr(ip_p, hdrwrds);
//   setiptos(ip_p, tos);
//   setipid(ip_p, id);
//   setipflags(ip_p, flags);
//   setipfoff(ip_p, foff);
//   setipttl(ip_p, ttl);
//   setipprot(ip_p, prot);
//   setipsrcip(ip_p, srcip);
//   setipdstip(ip_p, dstip);
//   setipdata(ip_p, ip_p->header + 20 * (sizeof(unsigned char)));
//   setiptlen(ip_p, tlen);
//   //if(hchksum == 0xFFFF) setiphchksum(ip_p, calculateipchecksum(ip_p));
// }

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

// // UDP source port
// int getudpsrcport(udpstruct_t *udp_p)
// {
//   return (udp_p->header[0] << 8) | (udp_p->header[1]);
// }

// void setudpsrcport(udpstruct_t *udp_p, int value)
// {
//   udp_p->header[0] = (unsigned char)(value >> 8);
//   udp_p->header[1] = (unsigned char)value;
// }

// // UDP destination port
// int getudpdstport(udpstruct_t *udp_p)
// {
//   return (udp_p->header[2] << 8) | (udp_p->header[3]);
// }

// void setudpdstport(udpstruct_t *udp_p, int value)
// {
//   udp_p->header[2] = (unsigned char)(value >> 8);
//   udp_p->header[3] = (unsigned char)value;
// }

// // UDP length
// int getudplen(udpstruct_t *udp_p)
// {
//   return (udp_p->header[4] << 8) | udp_p->header[5];
// }

// void setudplen(udpstruct_t *udp_p, int value)
// {
//   udp_p->header[4] = (unsigned char)(value >> 8);
//   udp_p->header[5] = (unsigned char)value;
// }

// int getudpchksum(udpstruct_t *udp_p)
// {
//   return (udp_p->header[6] << 8) | udp_p->header[7];
// }

// void setudpchksum(udpstruct_t *udp_p, int value)
// {
//   udp_p->header[6] = (unsigned char)(value >> 8);
//   udp_p->header[7] = (unsigned char)value;
// }

// // udp data
// unsigned char *getudpdata(udpstruct_t *udp_p)
// {
//   return udp_p->data;
// }

// void setudpdata(udpstruct_t *udp_p, unsigned char *data, int datacount)
// {
//   udp_p->data = udp_p->header + 8 * (sizeof(unsigned char)); //udp header is 8 bytes
//   //todo _Pragma
//   for (int i = 0; i < datacount; i++)
//   {
//     udp_p->data[i] = data[i];
//   }
// }

//*****************************************************************************
// RATIONALE
//*********************************************************************

// Rationale:
// The rationale in the stack is to focus on the 'MAX_networkbuf_NUM' networkbuffers. Each
// networkbuffer is of length 'MAX_networkbuf_SIZE' with type 'unsigned char' (same as 'uint8_t').
// There are structs for each layer in the stack (.., IP, UDP, ..). Each struct holds
// pointer(s) to places in the associated networkbuffer. The networkbuffer is not manipulated directly,
// but accessed with get/set functions that take a struct pointer as argument; this struct
// holds the addresses of the header and data. Both the networkbuffers and the structs are statically
// allocated. In example, udp[2], ip[2], and networkbuf[2] are connected by index 2.
//
// Toolchain:
// patmos-clang -O2 -mserialize=checksumwcet.pml checksumwcet.c
// platin wcet -i checksumwcet.pml -b a.out -e checksum --report
// checksum is the analysis entry point that would be inlined with -O2