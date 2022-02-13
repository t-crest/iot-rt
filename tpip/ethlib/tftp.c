#include "tftp.h"

//   Type   Op #     Format without header
//           2 bytes    string   1 byte     string   1 byte
//           -----------------------------------------------
//    RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
//    WRQ    -----------------------------------------------
//           2 bytes    2 bytes       n bytes
//           ---------------------------------
//    DATA  | 03    |   Block #  |    Data    |
//           ---------------------------------
//           2 bytes    2 bytes
//           -------------------
//    ACK   | 04    |   Block #  |
//           --------------------
//           2 bytes  2 bytes        string    1 byte
//           ----------------------------------------
//    ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
//           ----------------------------------------

unsigned char bufapp[100];
extern unsigned char my_ip[4];
// *********************** Build up the basic 5 TFTP responses *********************
// Opcode 01/02, RRQ/WRQ require
__attribute__((noinline)) 
int tpip_tftp_buildrwreq(unsigned char* udp_data, unsigned char op, unsigned char* filename, unsigned char* mode){
    int filename_length = 0;
    int mode_length = 0;
    int udplength = 0;
    unsigned char* temp_filename = filename;
    unsigned char* temp_mode = mode;
    
    *(unsigned short*)(udp_data) = 0x00 << 8 | op ;
    
    #pragma loopbound min 0 max 20
    while(*temp_filename != '\0') {
        filename_length++;
        temp_filename++;
    }
    #pragma loopbound min 0 max 8
    while(*temp_mode != '\0'){
        temp_mode++;
        mode_length++;
    }
    #pragma loopbound min 0 max 20
    for(int i =2; i < filename_length+2; i++){
        *(udp_data +i) = filename[i-2];
    }
    // printf("memcpy result filename: %s\n", udp_data);
    // printf("Filename length: %d \n", filename_length);
    // for(int i =0; i < filename_length+2;i++){
    //     printf("UDP DATA BEFORE LOADING: %d, %c", i ,udp_data[i]);
    // }

    udp_data[filename_length+2] = 0x00; 

    udplength = 2 + filename_length + 1;
    #pragma loopbound min 0 max 8
    for(int i = udplength; i < udplength + mode_length; i++){
        *(udp_data+i) = mode[i - udplength];
    } 
    udplength += mode_length;
    
    *(udp_data + udplength) = 0x00; udplength++;
    // printf("\n");
    // for(int i =0; i < udplength;i++){
    //     printf("UDP DATA BEFORE LOADING: %d, %02x", i ,udp_data[i]);
    // }
    return udplength;
}
// OPcode 03, DATA, transmission ends when it is less than 512 bytes 
__attribute__((noinline)) 
int tpip_tftp_builddata(unsigned char* udp_data, int blocknum, char* data){
    int length = 0;
    *((unsigned short*)udp_data) = 0x0003;              udp_data = udp_data+2;
    *((unsigned short*)udp_data) = blocknum & 0xFFFF;   udp_data = udp_data+2;
    #pragma loopbound min 0 max 50
    for(; data != NULL; udp_data++, data++){
        *udp_data = *data;
        length++;
    }

    return length;
}
// Opcode 04, ACK, has the same block number as the previous DATA block number
__attribute__((noinline)) 
void tpip_tftp_buildack(unsigned char* udp_data, int blocknum){
    *((unsigned int*)udp_data) = 0x00 << 24 | 0x04 << 16 | (blocknum & 0xffff);
}
// Opcode 05, ERROR 
__attribute__((noinline)) 
int tpip_tftp_builderror(unsigned char* udp_data, int errorcode, char* errormsg){
    int length = 0;
    *((unsigned short*)udp_data) = 0x0005;              udp_data = udp_data+2;
    *((unsigned short*)udp_data) = errorcode & 0xFFFF;  udp_data = udp_data+2;
    for(; *errormsg != '\0'; udp_data++, errormsg++){
        *udp_data = *errormsg;
        length++;
    }
    return length;
}

// ********************** PACKET OPERATIONS ***************
// Operation code
__attribute__((noinline)) 
unsigned short tpip_tftp_get_op(unsigned char* udp_data){
    return *((unsigned short*)udp_data);
}
// Block number
__attribute__((noinline)) 
unsigned short tpip_tftp_get_blocnum(unsigned char* udp_data){
    udp_data = udp_data + 2;
    return *((unsigned short*)udp_data);
}
// File name
__attribute__((noinline)) 
int tpip_tftp_get_filename(unsigned char* udp_data, char* filename){
    udp_data = udp_data + 2; //avoid the op
    int length = 0;
    while(*udp_data != 0x00){
        *filename = *udp_data;
        udp_data++; filename ++; length++;
    }
    return length;
}
// Mode
__attribute__((noinline)) 
int tpip_tftp_get_mode(unsigned char* udp_data, char* mode){
    char * filename;
    int mode_length = 0;
    mode_length = tpip_tftp_get_filename(udp_data, filename);
    udp_data = udp_data + mode_length + 2 + 1;
    mode_length = 0;
    // Before the devider symbol 0x00
    while(*udp_data != 0x00){
        *mode = *udp_data;
        udp_data++; mode ++; mode_length++;
    }

    return mode_length;
}
// DATA inside the packet
__attribute__((noinline)) 
int tpip_tftp_get_data(unsigned char* udp_data, char* data_buffer){
    udp_data = udp_data + 4; //avoid the op and blcnum
    int length = 0;
    #pragma loopbound min 0 max 512
    while(udp_data != NULL){
        *data_buffer = *udp_data;
        data_buffer++; udp_data++; length++;
    }

    return length;
}
// Error code
__attribute__((noinline)) 
unsigned short tpip_tftp_get_errcode(unsigned char* udp_data){
    udp_data = udp_data + 2;
    return *((unsigned short*)udp_data);
}
// Error Message
__attribute__((noinline)) 
int tpip_tftp_get_errmsg(unsigned char* udp_data, char* errormsg){
    udp_data = udp_data + 4; //avoid the op and blcnum
    int length = 0;
    while(*udp_data != 0x00){
        *errormsg = *udp_data;
        udp_data++; errormsg ++; length++;
    }

    return length;
}


// *********************** TFTP (receive/send data)/(reply ack) functions ********************
// Build IP header, length should be changed, and swap srcip and dstip
__attribute__((noinline)) 
void tpip_tftp_build_ipheader(ip_t* ip_send){
    ip_send->verhdl = 0x45;
    ip_send->tos    = 0x00;
    ip_send->length = 0x0014;
    ip_send->id     = 0x0001;
    ip_send->ff     = 0x4000;
    ip_send->ttl    = 0x40;
    ip_send->prot   = 0x11;
    ip_send->checksum = 0x0000;
    ip_send->srcip  = (192 << 24) | (168 << 16) | (24 << 8) | 50; // Patmos IP addr
    ip_send->dstip  = (192 << 24) | (168 << 16) | (24 << 8) | 10; // my linux machine
}
// Reply signal: ACK and ERROR
// Return a ACK
int tpip_tftp_process_reply_ack(ip_t* ip_recv,unsigned short srcport,unsigned char *bufin, unsigned char *bufout, unsigned int tx_addr){
    ip_t ip_send;
    ip_send.udp.data = bufout + 42;
    int load_success = 0;
    unsigned short blocknum = tpip_tftp_get_blocnum(ip_recv->udp.data);
    ip_send.udp.srcport = srcport;
    ip_send.udp.dstport = ip_recv->udp.srcport;
    ip_send.udp.checksum = 0;
    ip_send.udp.length = 8 + 4;
    tpip_tftp_buildack(ip_send.udp.data, blocknum);
    tpip_tftp_build_ipheader(&ip_send);
    // Ethernet Header. Swap MAC address 1-12bits
	for (int i=0; i<6; i++){
		*(bufout + i) = *(bufin + i + 6);
		*(bufout + i + 6) = *(bufin + i);
		printf("bufout[%d]: %d ",i,bufout[i]);
	}
    *((unsigned short*) bufout) = *((unsigned short*) bufin); // 13-14 bits 
    // Package into buffer
    packip(bufout+14, &ip_send);
    // UDP checksum
    udpchecksum(bufout+14);
    // IP checksum
    ipchecksum(bufout+14);
    // Load into EthMac 
    int frame_length = ip_send.length + 14;
    load_success = tpip_load_bufout(tx_addr, bufout, frame_length);
    // Send out
    if(load_success == 1) {
        eth_mac_send(tx_addr, frame_length);
        return 1;
    }
    else return 0;
}
// Send a read/write request to the server, after request udp packet built up
__attribute__((noinline)) 
int tpip_tftp_send_rrq(unsigned char op, unsigned char* filename, unsigned char* mode,unsigned char* bufout, unsigned int tx_addr){
    ip_t ip_send;

    int udp_data_length;
    int load_success = 0;
    unsigned char dst_mac[6]; unsigned char dst_ip[4];
    int arp_success = 0;
    //UDP DATA
    //printf("Build read request \n");
    //printf("Filename: %s \n", filename);
    ip_send.udp.data = bufapp;
    udp_data_length = tpip_tftp_buildrwreq(ip_send.udp.data, op, filename, mode);
    //printf("[Check] UDP DATA length: %d \n", udp_data_length);
   
    // UDP Header 
    ip_send.udp.srcport = 1235;
    ip_send.udp.dstport = TFTP_PORT;    // First Request sends to port 69 
    ip_send.udp.checksum = 0;
    ip_send.udp.length = 8 + udp_data_length;
    
    // IP Header
    tpip_tftp_build_ipheader(&ip_send);  
    // printf("verhdl: 0x%04x, tos: 0x%04x \n", ip_send.verhdl, ip_send.tos);
    // printf("length: 0x%04x\n", ip_send.length);
    ip_send.length = ip_send.length + ip_send.udp.length;
    //printf("\n[Check1] IP length: %d \n", ip_send.length);
    // printf("SRC IP: ");
    // tpip_print_ip(ip_send.srcip);
    // printf("DST IP: ");
    // tpip_print_ip(ip_send.dstip);
      //printf("verhdl: 0x%04x, tos: 0x%04x \n", ip_send.verhdl, ip_send.tos);
  //printf("length: 0x%04x\n", ip_send.length);

    // Search for destination MAC address in ARP table
    arp_uint2char(dst_ip, ip_send.dstip);
    if( (arp_success = arp_table_search(dst_ip, dst_mac)) == 0){
        // printf("\n[ERROR] DEST MAC ADDR IS NOT FOUND IN ARP TABLE! \n");
    }
        //printf("\n[Check2] IP length: %d \n", ip_send.length);
          //printf("verhdl: 0x%04x, tos: 0x%04x \n", ip_send.verhdl, ip_send.tos);
  //printf("length: 0x%04x\n", ip_send.length);

    // arp_table_print();
    // Ethernet Header.  MAC address 1-12bits
	for (int i=0; i<6; i++){
		*(bufout + i) = dst_mac[i]; // DEST MAC 
		*(bufout + i + 6) = my_mac[i]; // SRC MAC
		// printf("DEST MAC[%d]: 0x%02x ",i,bufout[i]);
	}
    *(bufout + 12) = 0x08; *(bufout + 13) = 0x00;
        //printf("\n[Check3] IP length: %d \n", ip_send.length);

    // Package into buffer
//     printf("\n[Check4] IP length: %d \n", ip_send.length);
//     printf("verhdl: 0x%04x, tos: 0x%04x \n", ip_send.verhdl, ip_send.tos);
//   printf("length: 0x%04x\n", ip_send.length);
// printf("***********************\n");
// printf("ip-t address bef: %p\n", &ip_send);

// printf("ip-t verhdl address bef: %p\n", &ip_send.verhdl);
    packip(bufout+14, &ip_send);
   
    // UDP checksum
    udpchecksum(bufout+14);
    // IP checksum
    ipchecksum(bufout+14);
    
    // Load into EthMac 
    int frame_length = ip_send.length + 14;
    //printf("\n frame length: %d \n", frame_length);
     //printf("BUffer[13]: 0x%04x\n ", *((unsigned short*) (bufout+12)));
    //printf("BUffer[14]: 0x%02x\n ", *(bufout+13));
    //printf("BUffer[15]: 0x%02x\n ", *(bufout+14));
    //printf("BUffer[16]: 0x%02x\n ", *(bufout+15));
    // printf("\n [CHECK] SRC IP:");
    // tpip_print_ip(ip_send.srcip);
    // printf("\n [CHECK] DST IP:");
    // tpip_print_ip(ip_send.dstip);
    load_success = tpip_load_bufout(tx_addr, bufout, frame_length);
    // Send out
    if(load_success == 1) {
        // printf("REQUEST IS READY FOR SENDING! \n");
        eth_mac_send(tx_addr, frame_length);
        return 1;
    }
    else return 0;

}
