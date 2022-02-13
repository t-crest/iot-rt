#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <machine/patmos.h>
#include "ethlib/eth_mac_driver.h"
#include "ethlib/eth_patmos_io.h"
#include "ethlib/mac.h"
#include "ethlib/tpip.h"
#include "ethlib/tpip_arp.h"
#include "ethlib/tftp.h"

#define LED (*((volatile _IODEV unsigned*)PATMOS_IO_LED ))

unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;
static unsigned char bufin[2000];
static unsigned char bufout[2000];
static unsigned char bufudp_recv[200];
static unsigned char bufudp_send[200];

unsigned short int led_udp_port = 1234;
unsigned short int my_udp_port = 1235;
extern unsigned char my_ip[4];
unsigned char dest_ip[4] = {192,168,24,10};
// First send a read request to the PC server, ask for a file, then reply to the 
// data transfer. Used operation: RRQ, DATA, ACK, ERROR. 
// In patmos client, it doesn't have to treat the RRQ/WRQ, ACK and ERR, only reply 
// DATA

void tftp_demo(){
    ip_t recv_ip_pck;
    arp_t recv_arp_packet;
    udp_t recv_udp_packet;
    unsigned short dest_port;
    unsigned short source_port;
    enum eth_protocol packet_type;
    int led_val;
    int udp_count = 0;
    int load_done = 0;
    int udp_data_length = 0;
    int blocknum = 0; // TFTP block number
    int tftp_data_count = 0;
    unsigned char ans;
    unsigned char filename[] = "hello.txt";
    unsigned char mode[]     = "netascii";
    int recv_success;

    eth_mac_initialize();
    arp_table_init();


    for(int i = 0; i < 20; i++){
        
        
        while(1){
            memset(bufin, 0, sizeof(bufin));
            memset(bufout, 0, sizeof(bufout));
            
            // Waiting for ARP TABLE update, send a TFTP read request to the server.
            arp_resolve_ip(bufin, bufout, rx_addr, tx_addr, dest_ip, 100000);
            printf("Send Read Request to the server \n");
            tpip_tftp_send_rrq(1, filename, mode, bufout, tx_addr);
            // ans = 0;
            
            // 1. receive the buffer: it should be loaded into ETH-MAC
            printf("\n Waiting for a packet, receive \n");
            eth_mac_receive(rx_addr,0); // 1us - 10ms???
            load_done = tpip_load_bufin(rx_addr, bufin);
            if(load_done == 0){
                printf("Nothing loaded into the buffer, waiting for next one! \n");
                continue;
            }
            // printf("Now data should be in the buffer, donot access HW again \n");
            packet_type = mac_packet_type(rx_addr);
            printf("TYPE OF SERVICE: %d \n", packet_type);

            if(packet_type == UNSUPPORTED){
                printf("\n- Level 3 protocol: UNKNOWN\n");
                printf("\n- Notes:\n");
                printf("  - No actions performed.\n");
            }
            else if(packet_type == ARP){
                printf("ARP RECEIVED \n");
                // Process the packet, reply or request
                ans = arp_process_received(bufin, bufout, tx_addr);                  
            }
            else if(packet_type == UDP){
                printf("Now the received data should be in the bufin \n");
                // 2. Extract the whole data from buffer to an IP packet
                unpackip(&recv_ip_pck, bufin+14); //Extract data from buffer                
                // Print whatever inside the IP packet
                printipdatagram(&recv_ip_pck); //Print them out
                printf("...packet #%d received!\n\n", i+1);
                printf("Packet #%d info:\n", i+1);
                printf("- Level 2 protocol: Ethernet\n");
                printf("\n- Level 3 protocol: IP\n");
                if((recv_ip_pck.dstip & 0xff) == 255){
                    printf(" [BROADCAST]");
                }

                // IP checksum
                int ip_checksum = tpip_verify_checksum(bufin);
                printf("\n  - IP checksum: %04x", recv_ip_pck.checksum);
                if(ip_checksum == 1){
                    printf(" [OK]\n");
                }else{
                    printf(" [WRONG]\n"); printf("\n- Notes:\n");
                    printf("  - Wrong IP checksum, no actions performed.\n");
                    continue;
                }

                //printf("UDP Data: 0x%02x \n", recv_ip_pck.udp.data[0]);
                
                // Start to process UDP message
                printf("\n- Level 4 protocol: UDP\n");
                printf("  - Source port: %d\n", recv_ip_pck.udp.srcport);
                printf("  - Destination port %d\n", recv_ip_pck.udp.dstport);
                printf("  - UDP checksum: %04X", recv_ip_pck.udp.checksum);
                // Sometimes the data part will get lost, do not know the reason, probably crosstalk
                            
                // VERIFY UDP checksum
                int udp_checksum = tpip_udp_verify_checksum(&recv_ip_pck); 
                if(udp_checksum == 1) printf(" [OK] \n");
                else printf(" UDP check sum [WRONG] \n");
                
                udp_data_length = recv_ip_pck.udp.length - 8;
                printf("  - Data length %d B\n", udp_data_length);
                // printf("Data: 0x%02x \n", recv_ip_pck.udp.data[0]);
                
                if ((recv_ip_pck.dstip & 0xFF) == 255){
                    printf("\n- Notes:\n");
                    printf("  - Broadcast UDP packet, no actions performed.\n");
                }else if (tpip_compare_ip(my_ip, recv_ip_pck.dstip) == 0){
                    printf("\n- Notes:\n");
                    printf("  - UDP packet not to our IP, no actions performed.\n");
                }else {
                    printf("\n- Notes:\n");
                    printf("  - UDP packet to our IP ");
                    
                    recv_udp_packet = recv_ip_pck.udp;
                    int op = tpip_tftp_get_op(recv_udp_packet.data);

                    if(recv_udp_packet.dstport == TFTP_PORT){ // request through 69 port
                        printf("\n- Notes:\n");
                        printf("  - TFTP RRQ/WRQ packet to our IP");                        
                        
                        int length = 0;
                        int tftp_data_length = 0;
                    
                        char* filename;
                        printf(" TFTP request opcode: %d \n", op);
                        switch (op)
                        {
                        case RRQ:
                            length = tpip_tftp_get_filename(recv_udp_packet.data, filename);
                            printf("Read request: %s length: %d\n", filename, length);
                            // Return data directly, no need for ack
                            //tpip_tftp_process_reply_data(&recv_ip_pck, bufout, tx_addr);
                        break;
                        
                        case WRQ:
                            length = tpip_tftp_get_filename(recv_udp_packet.data, filename);
                            printf("Read request: %s length: %d\n", filename, length);

                            // Return a ack reply with block number 0
                            //tpip_tftp_process_reply_ack(&recv_ip_pck, bufout, tx_addr);
                        break;
                        
                        default:
                            break;
                        }
                    }
                    else if(recv_udp_packet.dstport == my_udp_port){
                        if(op == DATA){
                            printf(" DATA received, store in external Flash \n");
                            blocknum = tpip_tftp_get_blocnum(recv_udp_packet.data);
                            udp_data_length = recv_udp_packet.length -8 -4;
                            printf("TFTP DATA Length: %d\n", udp_data_length);
                                                        
                            if(udp_data_length < 512){
                                printf(" DATA length < 512, store and end transmission \n");
                                for(int i = 0;i < tftp_data_count; i++){
                                    printf("tftp DATA: %d",recv_udp_packet.data[i]);
                                    // Store in a external RAM, have to check
                                    *(unsigned char*)(0x30000000 + 512*blocknum + i) = recv_udp_packet.data[i];
                                }
                                blocknum = 0; // reset the block num
                                break;
                            }
                            // The Max capacity of a TFTP data is 512 Bytes
                            #pragma loopbound min 0 max 512
                            for(int i = 0;i < 512; i++){
                                printf("tftp DATA: %d",recv_udp_packet.data[i]);                                
                                // Store in a external RAM, have to check
                                *(unsigned char*)(0x30000000 + 512*blocknum + i) = recv_udp_packet.data[i];
                            }                           
                        }
                        else if(op == ACK){
                            printf("ACK received, keep sending the next data \n");
                            blocknum++;
                            //tpip_tftp_process_reply_data(data,blocknum);

                        } 
                        else if(op == ERROR){
                            
                        }                          
                    }
                }            
            }
            else {
                printf("Other protocols that I don't care now. \n");
            }
        }
    
    }
    
    return;    
}



int main(){
    LED = 0X00;
    char c;
    printf("Test: TFTP + TPIP + ETH PROTOCOL \n");

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
            tftp_demo();
        }
    }

    return 0;
}