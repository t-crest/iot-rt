#ifndef _ICMP_H_
#define _ICMP_H_

#include <stdio.h>
#include "eth_patmos_io.h"
#include "eth_mac_driver.h"
#include "mac.h"
#include "tpip.h"

// ICMP: RFC792
// ONLY PING IS SUPPORTED
//                      Echo or Echo Reply Message
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |     Type      |     Code      |          Checksum             |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |           Identifier          |        Sequence Number        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |     Data ...
//    +-+-+-+-+-
typedef struct tpip_icmp
{
    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned short id;
    unsigned short seq_num;
    unsigned char* data;
}icmp_t;
typedef struct ip_icmp{
  unsigned char  verhdl;    // default ver. and header words (default 0x4 | 0x5)
  unsigned char  tos;       // default 0x00
  unsigned short length;    // total length (20 + udpmsg.length for UDP)
  unsigned short id;        // identification incremented on each new datagram
  unsigned short ff;        // (don't) fragment and fragment offset (default 010b | 0 => 0x4000)
  unsigned char  ttl;       // time to live (default 0x20)
  unsigned char  prot;      // protocol (default UDP 17 => 0x11)
  unsigned short checksum;  // default 0x0000
  unsigned int   srcip;     // src IP address
  unsigned int   dstip;     // dst IP address
  icmp_t* icmp;
} ip_icmp;

// Load icmp frames from RX buffer without the IP header 
void icmp_load(unsigned char* bufin, icmp_t* icmp_recv);
//The whole IP message
void icmp_load_ip(ip_icmp* icmp_pck, unsigned char* buf);

//This function takes the received ping request icmp paket starting in rx_addr 
//and builds a reply packet starting in tx_addr. rx_addr and tx_addr can be the same.
unsigned int icmp_build_ping_reply(unsigned char *bufin, unsigned char *bufout);

//This function process a received ICMP package. 
//If it is a ping request and we are the destination (IP) it reply the ping and returns 1. 
//Otherwise it returns 0.
int icmp_process_received(unsigned char* bufin, unsigned char* bufout, unsigned int tx_addr);

#endif
