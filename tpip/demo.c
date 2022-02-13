#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <machine/patmos.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/eth_patmos_io.h"
#include "ethlib/mac.h"
#include "ethlib/tpip.h"
#include "ethlib/tpip_arp.h"
#include "ethlib/tpip_icmp.h"

#define LED (*((volatile _IODEV unsigned*)PATMOS_IO_LED ))

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;
static unsigned char bufin[2000];
static unsigned char bufout[2000];

unsigned short int led_udp_port = 1234;


void demo(){
    ip_t recv_ip_pck;
    arp_t recv_arp_packet;
    ip_icmp icmp_recv;
    // unsigned char source_ip[4];
    // unsigned char source_mac[6];
    // unsigned char dest_ip[4];
    // unsigned char dest_mac[6];
    unsigned short dest_port;
    unsigned short source_port;
    enum eth_protocol packet_type;
    int led_val;
    int udp_count = 0;
    int load_done = 0;
    int udp_data_length = 0;
    int ip_checksum = 0;
   
    unsigned char ans;

    int recv_success;

    eth_mac_initialize();
    arp_table_init();

    for(int i = 0; i < 20; i++){
        ans =0;
        // 1. receive the buffer: it should be loaded into ETH-MAC
        printf("\n Waiting for a packet, receive \n");
       
        eth_mac_receive(rx_addr,0); // 1us - 10ms???
        load_done = tpip_load_bufin(rx_addr, bufin);
        if(load_done == 0){
            printf("Nothing loaded into the buffer, waiting for next one! \n");
            continue;
        }
        printf("Now data should be in the buffer, donot access HW again \n");
        packet_type = mac_packet_type(rx_addr);
        printf("TYPE OF SERVICE: %d \n", packet_type);
        switch (packet_type)
        {
        case UNSUPPORTED:
			printf("...packet #%d received!\n\n", i+1);
			printf("Packet #%d info:\n", i+1);
			printf("- Level 2 protocol: Ethernet\n");
			printf("\n- Level 3 protocol: UNKNOWN\n");
			printf("\n- Notes:\n");
			printf("  - No actions performed.\n");
	    break;

        case ICMP:  
            ans = icmp_process_received(bufin, bufout, tx_addr);
            icmp_load_ip(&icmp_recv, bufin+14);
            printf("...packet #%d received!\n\n", i+1);
            printf("Packet #%d info:\n", i+1);
            printf("- Level 2 protocol: Ethernet\n");
            printf("\n- Level 3 protocol: IP\n");
            
            printf("  - Source IP: ");
            tpip_print_ip(icmp_recv.srcip);
            printf("\n  - Destination IP: ");
            tpip_print_ip(icmp_recv.dstip);
            
            ip_checksum = tpip_verify_checksum(bufin);
            if(ip_checksum == 1){
                printf(" [OK]\n");
            }else{
                printf(" [WRONG]\n");
                printf("\n- Notes:\n");
				printf("  - Wrong IP checksum, no actions performed.\n");
				continue;
            }
            printf("\n- Level 3 protocol: ICMP\n");
            if (ans == 0){
                printf("\n- Notes:\n");
                printf("  - ICMP packet not our IP or not a ping request, no actions performed.\n");
            }else{
                printf("\n- Notes:\n");
                printf("  - Ping to our IP, replied.\n");
            }
        break;
        
        case UDP:
            printf("Now the received data should be in the bufin \n");
            // 2. Extract the whole data from buffer to an IP packet
            unpackip(&recv_ip_pck, bufin+14); //Extract data from buffer
            // Now this part it fixed, next step is to check the proper port
            // I think I should do the checksum and 

            // Print whatever inside the IP packet
            printipdatagram(&recv_ip_pck); //Print them out
            
            printf("...packet #%d received!\n\n", i+1);
            printf("Packet #%d info:\n", i+1);
            printf("- Level 2 protocol: Ethernet\n");
            printf("\n- Level 3 protocol: IP\n");
            
            if((recv_ip_pck.dstip & 0xff) == 255){
                printf(" [BROADCAST]");
            }
            // checksum
            
            printf("\n  - IP checksum: %04x", recv_ip_pck.checksum);
            ip_checksum = tpip_verify_checksum(bufin);
            if(ip_checksum == 1){
                printf(" [OK]\n");
            }else{
                printf(" [WRONG]\n");
                printf("\n- Notes:\n");
				printf("  - Wrong IP checksum, no actions performed.\n");
				continue;
            }
            printf("UDP Data: 0x%02x \n", recv_ip_pck.udp.data[0]);
            // Start to process UDP message
            printf("\n- Level 4 protocol: UDP\n");
			printf("  - Source port: %d\n", recv_ip_pck.udp.srcport);
			printf("  - Destination port %d\n", recv_ip_pck.udp.dstport);
			printf("  - UDP checksum: %04X", recv_ip_pck.udp.checksum);
            // Sometimes the data part will get lost, do not know the reason
            int udp_checksum = 0;
            udp_checksum = tpip_udp_verify_checksum(&recv_ip_pck); 
            if(udp_checksum == 1) printf(" [OK] \n");
            else printf(" UDP check sum [WRONG] \n");
            udp_data_length = recv_ip_pck.udp.length - 8;
            printf("  - Data length %d B\n", udp_data_length);
            printf("Data: 0x%02x \n", recv_ip_pck.udp.data[0]);
            if (tpip_udp_verify_checksum(&recv_ip_pck) == 0 || ip_checksum == 0){
				printf("\n- Notes:\n");
				printf("  - Wrong UDP checksum, no actions performed.\n");
			}else if ((recv_ip_pck.dstip & 0xFF) == 255){
				printf("\n- Notes:\n");
				printf("  - Broadcast UDP packet, no actions performed.\n");
			}else if (tpip_compare_ip(my_ip, recv_ip_pck.dstip) == 0){
				printf("\n- Notes:\n");
				printf("  - UDP packet not to our IP, no actions performed.\n");
            }else {
                printf("\n- Notes:\n");
				printf("  - UDP packet to our IP ");
				if(recv_ip_pck.udp.dstport == led_udp_port){
					printf("and with UDP destination port for LEDs.\n");
					//Drive the leds
					if (udp_data_length < 255){
						recv_ip_pck.udp.data[udp_data_length] = '\0';
						printf("  - The UDP data is: %s", recv_ip_pck.udp.data); // [print udp data as string]
						led_val = atoi((char *)recv_ip_pck.udp.data); // Convert a string to interger, is it necessary
						printf("  - The extracted value is %d. The lower byte is 0x%02X\n", led_val, (unsigned char) (led_val & 0xFF));
						LED = (unsigned char)(led_val & 0xFF);
					}else{
						printf("  - The UDP data exceed 256 characters.\n");
					}
                }
            }
        break;
        
        case ARP:
            printf("ARP RECEIVED \n");
			// Process the packet, reply or request
            ans = arp_process_received(bufin, bufout, tx_addr);
            // Let's see what is inside
			printf("...packet #%d received!\n\n", i+1);
			printf("Packet #%d info:\n", i+1);
			printf("- Level 2 protocol: Ethernet\n");
			printf("\n- Level 3 protocol: ARP\n");
			arp_unpack(&recv_arp_packet, bufin);

            arp_packet_print(&recv_arp_packet);
            // Print out MAC and IP address
			printf("  - Sender MAC: ");
			mac_print_mac(recv_arp_packet.S_HA);
			printf("\n  - Sender IP: ");
			tpip_print_ip(recv_arp_packet.S_L32);
			printf("\n  - Target MAC: ");
			mac_print_mac(recv_arp_packet.T_HA);
			printf("\n  - Target IP: ");
			tpip_print_ip(recv_arp_packet.T_L32);
			printf("\n  - Operation: 0x%04x ", recv_arp_packet.OP);
			if (recv_arp_packet.OP == 1){
				printf("[request]\n");
			}else if (recv_arp_packet.OP == 2){
				printf("[reply]\n");
			}else{
				printf("\n");
			}
			printf("\n- Notes:\n");
			if (ans == 0){
				printf("  - ARP request not to our IP, no actions performed.\n");
			}else if(ans == 1){
				printf("  - ARP request to our IP, replied.\n");
			}    
        break;
        
        default:
            printf("Other protocols that I don't care now. \n");
            break;
        }
    }
    
    return;    
}



int main(){
    //LED = 0X00;
    char c;
    printf("Test: TPIP + ETH PROTOCOL \n");

    memset(bufin, 0, sizeof(bufin));
    memset(bufout, 0, sizeof(bufout));

    while(1){
        printf("P: Get into loop \n");
        printf("Insert Num '3' to start the program \n");
        scanf(" %c", &c);
        if(c == 'q'){
           return 0;
        }
        while(c != '3'){
            printf("Waiting for start signal: ");
            scanf(" %c", &c);
            if(c == 'q')
                return 0;
        }
        if(c == '3'){
            demo();
        }
    }

    return 0;
}