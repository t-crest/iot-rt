/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/*
 * ARP section of ethlib (ethernet library)
 * 
 * Authors: Luca Pezzarossa (lpez@dtu.dk)
 *          Jakob Kenn Toft
 *          Jesper Lønbæk
 *          Russell Barnes
 */

#ifndef _ARP_H_
#define _ARP_H_

#include <stdint.h>
#include <stdio.h>
#include <machine/rtc.h>
#include "eth_patmos_io.h"
#include "mac.h"
#include "eth_mac_driver.h"
#include "tpip.h"

#define ARP_TABLE_SIZE 8

//APR datagram
  //       0        7        15       23       31
  //       +--------+--------+--------+--------+
  //       |       HT        |        PT       |
  //       +--------+--------+--------+--------+
  //       |  HAL   |  PAL   |        OP       |
  //       +--------+--------+--------+--------+
  //       |         S_HA (bytes 0-3)          |
  //       +--------+--------+--------+--------+
  //       | S_HA (bytes 4-5)|S_L32 (bytes 0-1)|
  //       +--------+--------+--------+--------+
  //       |S_L32 (bytes 2-3)|T_HA (bytes 0-1) |
  //       +--------+--------+--------+--------+
  //       |         T_HA (bytes 2-5)          |
  //       +--------+--------+--------+--------+
  //       |         T_L32 (bytes 0-3)         |
  //       +--------+--------+--------+--------+

typedef struct arp_t
{
  uint16_t HT;      //Hardware Type
  uint16_t PT;      //Protocol Type
  uint8_t  HAL;     //Hardware Address length
  uint8_t  PAL;     //Protocol Address Length
  uint16_t OP;      //Operation Code
  unsigned char S_HA[6]; //Sender MAC Address
  uint32_t S_L32;   //Sender IPV4 Address
  unsigned char T_HA[6];  //Target Hardware(MAC) Address
  uint32_t T_L32;   //Target IPV4 Address
}arp_t;

//This function gets everything from the input buffer to a structure
void arp_unpack(arp_t *arp, unsigned char *buf); 
// Pack up an ARP packet
void arp_pack(unsigned char *buf, arp_t *arp);
//Print out what we get inside arp packet
void arp_packet_print(arp_t *arp);
// Change uint32_t to unsigned char
void arp_uint2char(unsigned char *target, uint32_t ipnum);
// Print out the ARP packet
void arp_packet_print(arp_t *arp);
///////////////////////////////////////////////////////////////
//Functions related to the ARP table
///////////////////////////////////////////////////////////////

//This function initilize the ARP table.
void arp_table_init();

//This function searches in the ARP table for the given IP. If it exists it returns 1 and the MAC. If not it returns 0.
int arp_table_search(unsigned char ip_addr[], unsigned char mac_addr[]);

//This function remove ARP table entries for the given IP. If something is removed it returns 1. If not it returns 0.
int arp_table_delete_entry(unsigned char ip_addr[]);

//This function insert a new entry in the ARP IP/MAC table. If an entry was already there the fields are updated. If we are out of space, the first entry is replaced.
void arp_table_new_entry(unsigned char ip_addr[], unsigned char mac_addr[]);

//This ugly function prints the ARP table for debug purposes.
void arp_table_print();

///////////////////////////////////////////////////////////////
//Functions to get the ARP protocol field
///////////////////////////////////////////////////////////////


//              Data is already in the struct arp_t
//This function get the ARP target IP.
// void arp_get_target_ip(unsigned int pkt_addr, unsigned char target_ip[]);

//This function get the ARP target IP.
// void arp_get_target_mac(unsigned int pkt_addr, unsigned char target_mac[]);

//This function get the ARP target MAC.
// void arp_get_sender_ip(unsigned int pkt_addr, unsigned char sender_ip[]);

//This function get the ARP sender IP.
// unsigned short int arp_get_operation(unsigned int pkt_addr);

//This function get the ARP sender MAC.
// void arp_get_sender_mac(unsigned int pkt_addr, unsigned char sender_mac[]);

///////////////////////////////////////////////////////////////
//Support functions related to the ARP protocol
///////////////////////////////////////////////////////////////

//This function takes the received ARP request packet starting in rx_addr and builds a reply packet starting in tx_addr. rx_addr and tx_addr can be the same.
unsigned int arp_build_reply(unsigned char *bufin, unsigned char *bufout);

//This function process a received ARP package. If it is a request and we are the destination (IP) it reply the ARP request and returns 1. If it is a reply and we are the destination (IP) it inserts an entry in the ARP table and returns 2. Otherwise it returns 0.
int arp_process_received(unsigned char *bufin, unsigned char *bufout, unsigned int tx_addr);

//This function builds an ARP request packet starting at tx_addr and targeting the target_ip
unsigned int arp_build_request(unsigned int tx_addr, unsigned char target_ip[]);

//This function tries to resolves an ip address in the time specified by timeout (us). It waits for an answer for at least 100000us, hence if nobody replies it returns after 100000us, indipendently by the timeout. It requires a rx and a tx addr to send and receive packets.
int arp_resolve_ip(unsigned int rx_addr, unsigned int tx_addr, unsigned char target_ip[], long long unsigned int timeout);

#endif
