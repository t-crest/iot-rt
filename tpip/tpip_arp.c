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

#include "tpip_arp.h"


////////////////////////////////////////////////////
// Extract the ARP frame from receive buffer
////////////////////////////////////////////////////
void arp_unpack(arp_t *arp, unsigned char *buf){
  buf = buf + 14; // Avoid the Ethernet header
  arp->HT = *buf << 8 | *(buf+1); buf++; buf++;
  arp->PT = *buf << 8 | *(buf+1); buf++; buf++;
  arp->HAL = *buf; buf++;
  arp->PAL = *buf; buf++;
  arp->OP = *buf << 8 | *(buf+1); buf++; buf++;
  for(int i = 0; i < 6; i++){
    arp->S_HA[i] = *buf; 
    buf++;
  } 
  arp->S_L32 = *buf << 24 | *(++buf) << 16 | *(++buf) << 8 | *(++buf);
  buf++;
  for(int i = 0; i < 6; i++){
    arp->T_HA[i] = *buf; 
    buf++;
  } 
  arp->T_L32 = *buf << 24 | *(++buf) << 16 | *(++buf) << 8 | *(++buf);
  
}

//Print out the ARP packet
void arp_packet_print(arp_t *arp){
	printf("arp->HT    = 0x%04x\n", arp->HT);
	printf("arp->PT    = 0x%04x\n", arp->PT);
	printf("arp->HAL   = 0x%02x (%d)\n", arp->HAL, arp->HAL);
	printf("arp->PAL   = 0x%02x (%d)\n", arp->PAL, arp->PAL);
	printf("arp->OP    = 0x%04x\n", arp->OP);
	printf("arp->S_HA(Sender MAC)  = 0x");
	for(int i=0;i<6;i++){
		printf("%02x ", arp->S_HA[i]);
	}
	char arpstr[20]; 
	printipaddr(arp->S_L32, arpstr);
	printf("\n arp->S_L32(Sender IP)  = 0x%08lx (%s)\n", arp->S_L32, arpstr);
	printf("arp->T_HA(Target MAC)  = 0x");
	for(int i=0;i<6;i++){
		printf("%02x ", arp->T_HA[i]);
	}
	printipaddr(arp->T_L32, arpstr);
	printf("\n arp->T_L32(Target IP)  = 0x%08lx (%s)\n", arp->T_L32, arpstr);

	return;
}

void arp_pack(unsigned char *buf, arp_t *arp){
	
}
void arp_uint2char(unsigned char *target, uint32_t ipnum){
	for(int i = 0; i < 4; i++){
		target[i] = ipnum >> (24-8*i);
	}
	return;
}
///////////////////////////////////////////////////////////////
//Function supporting IPV4 function
///////////////////////////////////////////////////////////////
//This function compare 2 IPs. If they are the same, it returns 1, otherwise 0.
int ipv4_compare_ip(unsigned char ip1[], unsigned char ip2[]){
	if(ip1[0] == ip2[0] && ip1[1] == ip2[1] && ip1[2] == ip2[2] && ip1[3] == ip2[3]){
		return 1;
	}else{
		return 0;
	}
}
///////////////////////////////////////////////////////////////
//Functions related to the ARP table
///////////////////////////////////////////////////////////////

struct arp_table_entry{
	unsigned char used;//0 if empty 
	unsigned char ip_addr[4];
	unsigned char mac_addr[6];
};

struct arp_table_entry arp_table[ARP_TABLE_SIZE];

//This function initilize the ARP table.
void arp_table_init(){
	for (int i=0; i<ARP_TABLE_SIZE; i++){
		arp_table[i].used = 0;
	}
	return;
}

//This function searches in the ARP table for the given IP. If it exists it returns 1 and the MAC. If not it returns 0.
int arp_table_search(unsigned char ip_addr[], unsigned char mac_addr[]){
	#pragma loopbound min ARP_TABLE_SIZE max ARP_TABLE_SIZE
	for (int i=0; i<ARP_TABLE_SIZE; i++){
		if (arp_table[i].used == 1 && arp_table[i].ip_addr[0]==ip_addr[0] && arp_table[i].ip_addr[1]==ip_addr[1] && arp_table[i].ip_addr[2]==ip_addr[2] && arp_table[i].ip_addr[3]==ip_addr[3]){
			for(int j=0; j<6; j++){
				mac_addr[j] = arp_table[i].mac_addr[j];			
			}
			return 1;
		}
	}
	return 0;
}

//This function remove ARP table entries for the given IP. If something is removed it returns 1. If not it returns 0.
int arp_table_delete_entry(unsigned char ip_addr[]){
	int ans = 0;	
	#pragma loopbound min ARP_TABLE_SIZE max ARP_TABLE_SIZE
	for (int i=0; i<ARP_TABLE_SIZE; i++){
		if (arp_table[i].used == 1 && arp_table[i].ip_addr[0]==ip_addr[0] && arp_table[i].ip_addr[1]==ip_addr[1] && arp_table[i].ip_addr[2]==ip_addr[2] && arp_table[i].ip_addr[3]==ip_addr[3]){
			arp_table[i].used=0;
			ans = 1;
		}
	}
	return ans;
}

//This function insert a new entry in the ARP IP/MAC table. If an entry was already there the fields are updated. If we are out of space, the first entry is replaced.
void arp_table_new_entry(unsigned char ip_addr[], unsigned char mac_addr[]){
	unsigned char previous_mac[6];
	int i = 0;
	if(arp_table_search(ip_addr, previous_mac) == 1){
		arp_table_delete_entry(ip_addr);
	}
	#pragma loopbound min 1 max ARP_TABLE_SIZE
	while(arp_table[i].used == 1 && i<ARP_TABLE_SIZE){
		i++;
	}
	if (i == ARP_TABLE_SIZE){
		i = 0;//table is full
	}
	arp_table[i].used = 1;
	#pragma loopbound min 4 max 4
	for(int j=0; j<4; j++){
		arp_table[i].ip_addr[j] = ip_addr[j];
	}
	#pragma loopbound min 6 max 6
	for(int j=0; j<6; j++){
		arp_table[i].mac_addr[j] = mac_addr[j];
	}
	return;

}

//This ugly function prints the ARP table for debug purposes.
void arp_table_print(){
	printf("ARP IP/MAC table\n#\tUsed\tIP\t\tMAC\n");
	for (int i=0; i<ARP_TABLE_SIZE; i++){
		printf("%d\t%d\t%d.%d.%d.%d\t", i, arp_table[i].used, arp_table[i].ip_addr[0], arp_table[i].ip_addr[1], arp_table[i].ip_addr[2], arp_table[i].ip_addr[3]);
		printf("%02X:%02X:%02X:%02X:%02X:%02X\n", arp_table[i].mac_addr[0], arp_table[i].mac_addr[1], arp_table[i].mac_addr[2], arp_table[i].mac_addr[3], arp_table[i].mac_addr[4], arp_table[i].mac_addr[5]);
	}
	return;
}


///////////////////////////////////////////////////////////////
//Support functions related to the ARP protocol
///////////////////////////////////////////////////////////////

//This function takes the received ARP request packet starting in rx_addr and builds 
//a reply packet starting in tx_addr. rx_addr and tx_addr can be the same.
unsigned int arp_build_reply(unsigned char *bufin, unsigned char *bufout){

	unsigned int frame_length = 42;//ARP frames have a fixed length, ARP+ETH header
	
	//Copy the entire frame	
	for(int i=0; i<frame_length; i++){
		*(bufout+i) = *(bufin+i); 
	}
	
	//Swap MAC addrs in ethernet header
	for (int i=0; i<6; i++){
		*(bufout + i) = *(bufout + i + 6);
		*(bufout + i + 6) = my_mac[i];
		printf("bufout[%d]: %d ",i,bufout[i]);
	}
	printf("\n");
	//Change ARP Operation type from request to reply
	*(bufout + 21) = 2;
	//Swap MAC addrs
	for (int i=0; i<6; i++){
		*(bufout + 32 + i) = *(bufout + 22 + i);
		*(bufout + 22 + i) = my_mac[i];
	}
	//Swap IP addr
	for (int i=0; i<4; i++){
		bufout[38+i] = bufout[28+i];
		bufout[28+i] = my_ip[i];
	}
	return frame_length;
}

//This function process a received ARP package. If it is a request and we are the destination (IP) 
//it reply the ARP request and returns 1. If it is a reply and we are the destination (IP) it inserts 
//an entry in the ARP table and returns 2. Otherwise it returns 0.
int arp_process_received(unsigned char *bufin, unsigned char *bufout, unsigned int tx_addr){
	//Check if we are the destnation (IP)
	arp_t arp;
	int load_success = 0;
	unsigned char target_ip[4];
	unsigned char source_ip[4];
	arp_unpack(&arp, bufin);
	arp_uint2char(target_ip, arp.T_L32);
	arp_uint2char(source_ip, arp.S_L32);
	printf("TARGET IP: ");
	for(int i = 0; i < 4; i++){
		printf(" %d ", target_ip[i]);
	}
	if (ipv4_compare_ip(target_ip, my_ip) == 1){
		//Check if it is a arp request
		if (arp.OP == 0x01){		
			unsigned int frame_length = arp_build_reply(bufin, bufout);
			printf("\n ARP frame length: %d\n", frame_length);
			// Load data from buffer out to the HW buffer
			load_success = tpip_load_bufout(tx_addr, bufout, frame_length);
			if(load_success == 1) {
				eth_mac_send(tx_addr, frame_length);
				return 1;
			}
			else return 0;
			
		}else if (arp.OP == 0x02){ // ARP is a reply
			if(mac_compare_mac(arp.T_HA, my_mac) == 1){
				//insert an entry
				arp_table_new_entry(source_ip, arp.S_HA);
				return 2;
			}else{
				return 0;
			}
		}
	}	
	return 0;
}

//This function builds an ARP request packet starting at tx_addr and targeting the target_ip
// unsigned int arp_build_request(unsigned int tx_addr, unsigned char target_ip[]){

// 	unsigned int frame_length = 42;//ARP frames have a fixed length
	
// 	//MAC addrs
// 	#pragma loopbound min 6 max 6
// 	for (int i=0; i<6; i++){
// 		mem_iowr_byte(tx_addr + i, 0xFF);//ETH header broadcast
// 		mem_iowr_byte(tx_addr + 6 + i, my_mac[i]);//ETH header mymac
// 		mem_iowr_byte(tx_addr + 22 + i, my_mac[i]);//Sender mymac
// 		mem_iowr_byte(tx_addr + 32 + i, 0x00);//Clear target
// 	}
// 	//IP addrs
// 	#pragma loopbound min 4 max 4
// 	for (int i=0; i<4; i++){
// 		mem_iowr_byte(tx_addr + 28 + i, my_ip[i]);//Sender myip
// 		mem_iowr_byte(tx_addr + 38 + i, target_ip[i]);//Target ip
// 	}
// 	//ETH header type + Hardware type
// 	mem_iowr(tx_addr + 12, 0x08060001);
// 	//Protocol type + HLEN + PEN
// 	mem_iowr(tx_addr + 16, 0x08000604);
// 	//Operation
// 	mem_iowr_byte(tx_addr + 20, 0x00);
// 	mem_iowr_byte(tx_addr + 21, 0x01);
// 	return frame_length;
// }

// //This function tries to resolves an ip address in the time specified by timeout (us). It waits for an answer for at least 100000us, hence if nobody replies it returns after 100000us, indipendently by the timeout. It requires a rx and a tx addr to send and receive packets.
// int arp_resolve_ip(unsigned int rx_addr, unsigned int tx_addr, unsigned char target_ip[], long long unsigned int timeout){
// 	unsigned char ans = 0;
// 	unsigned int frame_length;
// 	unsigned long long int start_time = get_cpu_usecs();
// 	do{
// 		frame_length = arp_build_request(tx_addr, target_ip);
// 		eth_mac_send(tx_addr, frame_length);
// 		if (eth_mac_receive(rx_addr, 100000) == 1){
// 			if (mac_packet_type(rx_addr) == ARP){
// 				if (arp_process_received(rx_addr, tx_addr) == 2){
// 					//Something was inserted in the ARP table
// 					unsigned char tmp_mac[6];
// 					if (arp_table_search(target_ip, tmp_mac) == 1){
// 					ans = 1;
// 					}
// 				}
// 			}
// 		}
// 	}while ((ans == 0) && (get_cpu_usecs()-start_time < timeout));
// 	return ans;
// }

