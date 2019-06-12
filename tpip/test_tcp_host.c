/*
	Demo file for TCP

	Author: Eleftherios Kyriakakis 
	Copyright: DTU, BSD License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memset
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BROADCAST_MAC (unsigned char[6]) {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
#define TASK_PERIOD 2000 //us is 60Hz 
#define PORT 666

static char recv_buffer[1024] = {0};

struct sockaddr_in serv_addr;

int tcp_server_run(){
    char *hello = "Hello from server";
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
    valread = read( new_socket , recv_buffer, 1024); 
    puts(recv_buffer);
    puts("Replying:\n"); 
    send(new_socket , hello , strlen(hello) , 0 ); 
    printf("Hello message sent\n");
    close(new_socket);
    close(server_fd);
    return 0;
}

int tcp_client_run(){
    char *hello = "Hello from Patmos";
    unsigned char target_ip[INET_ADDRSTRLEN] = "192.168.2.254";
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
    valread = read( sock , recv_buffer, 1024); 
    puts(recv_buffer); 
    close(sock);
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
