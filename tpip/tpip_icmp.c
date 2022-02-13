#include "tpip_icmp.h"


///////////////////////////////////////////////////////////////
//Support functions related to the ICMP protocol
///////////////////////////////////////////////////////////////
__attribute__((noinline)) 
void icmp_load(unsigned char* bufin, icmp_t* icmp_recv){
    //bufin += 34; //Avoid headers
    icmp_recv->type     = *bufin;
    // printf("type: %d\n", icmp_recv->type);
    icmp_recv->code     = *(bufin + 1);
    // printf("code: %d\n", icmp_recv->code);
    icmp_recv->checksum = *(unsigned short*)(bufin + 2);
    icmp_recv->id       = *(unsigned short*)(bufin + 4);
    icmp_recv->seq_num  = *(unsigned short*)(bufin + 6);
    // printf("seq_num: %d\n", icmp_recv->seq_num);
    icmp_recv->data     = (bufin + 8);
    return;
}
__attribute__((noinline)) 
void icmp_load_ip(ip_icmp* icmp_pck, unsigned char* buf){
    // printf("load the whole ip packet \n");
    icmp_pck->verhdl = *buf; buf++;
    icmp_pck->tos = *buf; buf++;
    icmp_pck->length = ntohs(*((unsigned short*)(buf))); buf++; buf++;
    icmp_pck->id = ntohs(*((unsigned short*)(buf))); buf++; buf++;
    icmp_pck->ff = ntohs(*((unsigned short*)(buf))); buf++; buf++;
    icmp_pck->ttl = *buf; buf++;
    icmp_pck->prot = *buf; buf++;
    icmp_pck->checksum = ntohs(*((unsigned short*)(buf))); buf++; buf++;
    icmp_pck->srcip = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; buf += 4; 
    icmp_pck->dstip = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; buf += 4;
    // tpip_print_ip(icmp_pck->srcip);
    // tpip_print_ip(icmp_pck->dstip);

    // printf("where is it stopped?\n");
    //buf++;
    icmp_load(buf, icmp_pck->icmp);
    return;
}
//This function takes the received ping request icmp paket starting in rx_addr 
//and builds a reply packet starting in tx_addr. rx_addr and tx_addr can be the same.
__attribute__((noinline)) 
unsigned int icmp_build_ping_reply(unsigned char *bufin, unsigned char *bufout){
    // printf("build up ping reply");
	unsigned int frame_length = 14 + (bufin[16] << 8 | bufin[17]);
	
    //Copy the entire frame	
    #pragma loopbound min 0 max 50
	for(int i=0; i<frame_length; i++){
		*(bufout+i) = *(bufin+i); 
	}
	
	//Swap MAC addrs in ethernet header
	for (int i=0; i<6; i++){
		*(bufout + i) = *(bufin + i + 6);
		*(bufout + i + 6) = my_mac[i];
		// printf("bufout[%d]: 0x%02x ",i,bufout[i]);
	}
	// printf("\n");
	
	//Swap IP addr
	for (int i=0; i<4; i++){
		bufout[30+i] = bufin[26+i];
		bufout[26+i] = my_ip[i];
        // printf("IP bufout[%d]: 0x%d ",i,bufout[30+i]);
	}
    //Change ICMP type from request to reply
	bufout[34] = 0x00;

	//Change ICMP checksum
	unsigned short int checksum;
	checksum = bufin[36]<<8 | bufin[37];
    // printf("checksum before: 0x%04x\n", checksum);
	checksum = ~((~checksum) - 0x0800);
    // printf("checksum after: 0x%04x\n", checksum);
	bufout[36]= checksum>>8 ;//hi byte
	bufout[37]= checksum & 0xff;//lo byte

	return frame_length;
}

//This function process a received ICMP package. 
//If it is a ping request and we are the destination (IP) it reply the ping and returns 1. 
//Otherwise it returns 0.
int icmp_process_received(unsigned char* bufin, unsigned char* bufout, unsigned int tx_addr){
    icmp_t icmp_pck;
    icmp_load(bufin+34, &icmp_pck);
    printf("Type: %d \n", icmp_pck.type);
    printf("Code: %d \n", icmp_pck.code);
    //Check if it is a ping request
	if (icmp_pck.type == 0x08){
		//Check if we are the destnation (IP)
		unsigned int destination_ip = *(bufin + 30) << 24 | *(bufin + 31) << 16 | *(bufin + 32) << 8 | *(bufin + 33);
        printf("icmp receive dest ip: ");
        tpip_print_ip(destination_ip);
		if (tpip_compare_ip(my_ip, destination_ip)){
			unsigned int frame_length = icmp_build_ping_reply(bufin, bufout);
			printf("Frame Length: %d \n", frame_length);
            tpip_load_bufout(tx_addr, bufout, frame_length);
            eth_mac_send(tx_addr, frame_length);
			return 1;
		}	
	}
	return 0;
}
