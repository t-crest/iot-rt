#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <arpa/tftp.h>

int main(){
    // Socket create
    int network_socket;
    network_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(network_socket == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Define an Address for the socket to connect with
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.24.50");
    server_address.sin_port = htons(69);

    int connection_status = connect(network_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if(connection_status == -1){
        printf("False to connect \n");
    }

    char server_response[512];
    recv(network_socket, &server_response,sizeof(server_response), 0);

    printf("The server received data: %s \n", server_response);
    close(network_socket);
    return 0;
}

int tpip_server(){
    FILE* fp;
    fp = fopen("hello.txt","r+");
    char server_buffer[512];
    size_t data_length = fread(server_buffer, sizeof(char), 512, fp);
    
    int network_socket;
    network_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(network_socket == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Define an Address for the socket to connect with
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.24.50"); // INADDR_ANY
    server_address.sin_port = htons(69);

    bind(network_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(network_socket, 3);

    int client_socket;
    client_socket = accept(network_socket,NULL,NULL);

    send(client_socket, server_buffer, sizeof(server_buffer), 0);

    close(network_socket);
    return 0;
}