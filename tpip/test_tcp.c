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
	Demo file for TCP

	Author: Eleftherios Kyriakakis 
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memset

// #define __PATMOS__

#if defined(__PATMOS__)
#include <machine/patmos.h>
#include <machine/spm.h>
#include "tcp.h"
#include "eth_mac_driver.h"
#define TASK_PERIOD 16666 //us -> 60Hz 
volatile _SPM int *led_ptr  = (volatile _SPM int *) 0xF0090000;
unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
struct sockaddr_in serv_addr;
#endif

/* 
 *
 * Declaring Flags & Constants
 * 
 */
// #define SERVER
// #define CLIENT

#define BROADCAST_MAC (unsigned char[6]) {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
#define PORT 666

static char buffer[1024] = {0};

int tcp_server_run(){
    char *hello = "Hello from server";
    #if defined(__PATMOS__)
    return 0;
    #else
    int server_fd, new_socket, valread; 
    int opt = 1; 
    int addrlen = sizeof(serv_addr); 
       
    // Creating socket file descriptor 
    puts("Creating socket...\n");
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port
    puts("Attaching to port...\n");
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "192.168.2.50", &serv_addr.sin_addr)<=0)  
    { 
        puts("invalid serv_addr\n"); 
        return -1; 
    } 
       
    // Forcefully attaching socket to the port 
    puts("Binding...\n");
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    puts("Listening...\n");
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen failed"); 
        exit(EXIT_FAILURE); 
    }
    puts("Waiting to accept...\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t*)&addrlen))<0) 
    { 
        perror("accept failed"); 
        exit(EXIT_FAILURE); 
    }
    puts("Receiving:\n");
    valread = read( new_socket , buffer, 1024); 
    puts(buffer);
    puts("Replying:\n"); 
    send(new_socket , hello , strlen(hello) , 0 ); 
    printf("Hello message sent\n");
    close(new_socket);
    close(server_fd);
    #endif
    return 0;
}

int tcp_client_run(){
    char *hello = "Hello from Patmos";
    #if defined(__PATMOS__)
    static unsigned char target_ip[4] = {192, 168, 2, 50};
    static unsigned char target_mac[6] = {0x80, 0xce, 0x62, 0xd8, 0xc7, 0x39};
    static unsigned char spoof_mac[6] = {0xb8, 0x27, 0xeb, 0x5d, 0x63, 0xdf};
    unsigned char connect_attempts = TCP_SYN_RETRIES;	
    unsigned char resolved = 0;
    tcp_connection conn;
    //MAC controller settings
    eth_iowr(0x40, 0xEEF0DA42);
    eth_iowr(0x44, 0x000000FF);
    eth_iowr(0x00, 0x0000A423);
    ipv4_set_my_ip((unsigned char[4]){192, 168, 2, 1});
    tcp_init_connection(&conn, my_mac, target_mac, (unsigned char[4]){192, 168, 2, 1}, target_ip, 40106, PORT);
    puts("Connecting...\n");
    do{
        tcp_connect(tx_addr, rx_addr, &conn);
        eth_mac_receive(rx_addr, TCP_RETRY_INTERVAL);
        if(mac_packet_type(rx_addr)==ARP){
            arp_process_received(rx_addr, tx_addr);
            *led_ptr = 0xF0;
            continue;
        } else if(mac_packet_type(rx_addr)==TCP) {
            resolved = tcp_handle(tx_addr, rx_addr, &conn, (unsigned char*) "", 0);
            *led_ptr = resolved;
        }
        *led_ptr = connect_attempts;
        connect_attempts--;
    }while(conn.status != ESTABLISHED && connect_attempts > 0);
    if(conn.status == ESTABLISHED)
    {
        puts("Sending...\n");
        do{
            tcp_push(tx_addr, rx_addr, &conn, (unsigned char*) hello, strlen(hello));
            eth_mac_receive(rx_addr, TCP_RETRY_INTERVAL);
            if(mac_packet_type(rx_addr)==ARP){
                arp_process_received(rx_addr, tx_addr);
                *led_ptr = 0xF0;
                continue;
            } else if(mac_packet_type(rx_addr)==TCP) {
                resolved = tcp_handle(tx_addr, rx_addr, &conn, (unsigned char*) "", 0);
                *led_ptr = resolved;
            }
        }while(!resolved);
        puts("Hello message sent\n"); 
        puts("Receiving:\n");
        do{
            eth_mac_receive(rx_addr, TCP_RETRY_INTERVAL);
            if(mac_packet_type(rx_addr)==ARP){
                arp_process_received(rx_addr, tx_addr);
                *led_ptr = 0xF0;
                continue;
            } else if(mac_packet_type(rx_addr)==TCP) {
                resolved = tcp_handle(tx_addr, rx_addr, &conn, (unsigned char*) buffer, 1024);
                *led_ptr = resolved;
            }
        }while(!resolved);
        puts((char*)buffer);
        do{
            tcp_close(tx_addr, rx_addr, &conn);
            eth_mac_receive(rx_addr, TCP_RETRY_INTERVAL);
            if(mac_packet_type(rx_addr)==ARP){
                arp_process_received(rx_addr, tx_addr);
                eth_mac_receive(rx_addr, TCP_RETRY_INTERVAL);
                *led_ptr = 0xF0;
                continue;
            } else if(mac_packet_type(rx_addr)==TCP) {
                resolved = tcp_handle(tx_addr, rx_addr, &conn, (unsigned char*) buffer, 1024);
                *led_ptr = resolved;
            }
        }while(!resolved);
    } else if(connect_attempts == 0 && conn.status != ESTABLISHED) {
        puts("Failed to connect after max retries.\n");
        *led_ptr = 0x88;
    }
    #else
    unsigned char target_ip[INET_ADDRSTRLEN] = "192.168.2.50";
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    puts("Creating socket...\n");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        puts("\n socket error \n"); 
        return -1; 
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
        
    // Convert IPv4 and IPv6 serv_addres from text to binary form 
    if(inet_pton(AF_INET, target_ip, &serv_addr.sin_addr)<=0)  
    { 
        puts("invalid serv_addr\n"); 
        return -1; 
    } 

    puts("Connecting...\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        puts("connection failed \n"); 
        return -1; 
    }
    puts("Sending...\n");
    send(sock , hello , strlen(hello) , 0 ); 
    puts("Hello message sent\n"); 
    puts("Receiving:\n");
    valread = read( sock , buffer, 1024); 
    puts(buffer); 
    close(sock);
    #endif
    return 0;
}

int main(){
    int ret;
    //Demo
    #if defined(SERVER)
    puts("\nTCP Server Demo Started");
    ret = tcp_server_run();
    puts("\nTCP Server Demo Finished");
    #endif
    #if defined(CLIENT)
    puts("\nTCP Client Demo Started");
    ret = tcp_client_run();
    puts("\nTCP Client Demo Finished");
    #endif
    return ret;
}
