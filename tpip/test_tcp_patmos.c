/*
	Demo file for TCP

	Author: Eleftherios Kyriakakis 
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memset
#include <machine/patmos.h>
#include <machine/spm.h>
#include "tcp.h"
#include "eth_mac_driver.h"

#define BROADCAST_MAC (unsigned char[6]) {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
#define TASK_PERIOD 2000 //us is 60Hz 
#define PORT 666

static char recv_buffer[1024] = {0};

enum appstate{CONNECTING=0x0, SENDING=0x1, RECEIVING=0x3, DISCONNECT=0x7, DONE=0xF};
volatile _SPM int *led_ptr  = (volatile _SPM int *) 0xF0090000;
unsigned int rx_addr = 0x000;
unsigned int tx_addr = 0x800;
int checkForPacket(){
    int resolved = 0;
    eth_mac_receive_nb(rx_addr);
    if(mac_packet_type(rx_addr)==ARP){
        arp_process_received(rx_addr, tx_addr);
        return ARP;
    }
    return 0;
}

int tcp_server_run(){
    char *hello = "Hello from server";
    return 0;
}

int tcp_client_run(){
    char *hello = "Hello from Patmos";
    static unsigned char my_ip[4] = {192, 168, 2, 2};
    static unsigned char target_ip[4] = {192, 168, 2, 254};
    static unsigned char target_mac[6] = {0x80, 0xce, 0x62, 0xd8, 0xc7, 0x39};
    unsigned char connect_attempts = TCP_SYN_RETRIES;
    enum appstate clientstate = CONNECTING;
    tcp_connection connA;
    int iteration = 0;
    //MAC controller settings
    eth_mac_initialize();
    ipv4_set_my_ip(my_ip);
    printf("MAC: inti'd\n");
    printf("ARP: init'd\n");
    //Initialize connection
    tcp_init_connection(&connA, tx_addr, rx_addr, my_mac, target_mac, my_ip, target_ip, 40106, PORT, 17, 256);
    printf("TCP: init'd\n");
    //Start the timer
    unsigned int start_time = get_cpu_usecs();
    unsigned int elapsed_time = start_time;
    //Non-blocking main loop
	while(1){
        elapsed_time = get_cpu_usecs()-start_time;
        if(elapsed_time >= TASK_PERIOD){
            start_time = get_cpu_usecs();
            switch(clientstate){
                case CONNECTING:
                    if(connect_attempts > 0){
                        puts("Connecting attempt...\n");
                        if(tcp_connect(&connA) == 1){
                            clientstate = SENDING;
                        } else {
                            --connect_attempts;
                        }
                    } else {
                        printf("Connection failed after %d attempts\n", TCP_SYN_RETRIES);
                        return -1;
                    }
                    break;
                case SENDING:
                    connA.send_buffer = (unsigned char*) hello;
                    printf("Sending msg: %s\n", connA.send_buffer);
                    if(tcp_push(&connA) == 1){
                        clientstate = RECEIVING;
                    }
                    break;
                case RECEIVING:
                    puts("Receiving:");
                    if(tcp_recv(&connA) == 1){
                        puts((char*) connA.recv_buffer);
                        clientstate = DISCONNECT;
                    }
                    break;
                case DISCONNECT:
                    puts("Closing...\n");
                    if(tcp_close(&connA) == 1){
                        clientstate = DONE; 
                    }
                    break;
                case DONE:
                    puts("Exiting...\n");
                    return 0;
            };
            *led_ptr = ~*led_ptr & clientstate;
        }
        checkForPacket();
    }
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
