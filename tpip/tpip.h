/*
  tpIP, the time-predictable TCP/IP stack.

  Copyright: DTU, CBS
  Authors: Martin Schoeberl, Rasmus Ulslev Pedersen
  License: Simplified BSD
*/
#ifndef TPIP_H
#define TPIP_H

// network buffer for one packet
#define BUFSIZEWORDS (512 / 4)

// UDP header: https://www.ietf.org/rfc/rfc768
//  0      7 8     15 16    23 24    31
//                +--------+--------+--------+--------+
//                |     Source      |   Destination   |
//                |      Port       |      Port       |
//                +--------+--------+--------+--------+
//                |                 |                 |
//                |     Length      |    Checksum     |
//                +--------+--------+--------+--------+
//                |
//                |          data octets ...
//                +---------------- ...

// udp message
typedef struct udp_t
{
  unsigned short srcport;  // default lowest two bytes of src IP
  unsigned short dstport;  // default lowest two bytes of dst IP
  unsigned short length;   // default 8 bytes + 4 bytes (for a one data word test)
  unsigned short checksum; // default 0x0000
  unsigned char *data;     // must be modolu 4
} udp_t;

//  IP header: https://tools.ietf.org/html/rfc791
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |Version|  IHL  |Type of Service|          Total Length         |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |         Identification        |Flags|      Fragment Offset    |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |  Time to Live |    Protocol   |         Header Checksum       |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                       Source Address                          |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                    Destination Address                        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                    Options                    |    Padding    |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | Data...                                                       |

// ip datagram
typedef struct ip_t
{
  unsigned char verhdl;    // default ver. and header words (default 0x4 | 0x5)
  unsigned char tos;       // default 0x00
  unsigned short length;   // total length (20 + udpmsg.length for UDP)
  unsigned short id;       // identification incremented on each new datagram
  unsigned short ff;       // (don't) fragment and fragment offset (default 010b | 0 => 0x4000)
  unsigned char ttl;       // time to live (default 0x20)
  unsigned char prot;      // protocol (default UDP 17 => 0x11)
  unsigned short checksum; // default 0x0000
  unsigned long srcip;     // src IP address
  unsigned long dstip;     // dst IP address
  udp_t udp;               // default ip data paylod: udp message
} ip_t;


// proposal for using types (MS)
// we cannot safely use shorter fields due to
// possible different byte orders.
// And copy stuff around is not so efficient.
// Those fields referencing 32-bit words and include
// all subfields. E.g., length word includes all the
// subfields:
//  |Version|  IHL  |Type of Service|          Total Length         |

// It is "just" a naming instead of accessing a buff[0].


typedef struct ip_t_ms {
  unsigned long length;
  //  |Version|  IHL  |Type of Service|          Total Length         |
  unsigned long id;
//  |         Identification        |Flags|      Fragment Offset    |
  unsigned long prot;
  unsigned long source;
  unsigned long destination;
} ip_t_; // there was a name collistion with the other ip_t above

// as an example to get the version:
// ntohl(ipbuf.length) >> 28

// loads a network buffer with an ip packet
// first word is how many words in use
int packip(unsigned long networkbuf[], const ip_t *ip);

// unloads an ip packet from a network buffer
// first word is how many words in use
void unpackip(ip_t *ip, const unsigned int networkbuf[]);

// print ip address
void printip(unsigned long ip);

#ifndef __patmos__
#include <arpa/inet.h>
#else
//not implemented on patmos
inline unsigned int htonl(unsigned int val) { return val; }
inline unsigned short htons(unsigned short val) { return val; }
inline unsigned int ntohl(unsigned int val) { return val; }
inline unsigned short ntohs(unsigned short val) { return val; }
#endif

#endif // TPIP_H