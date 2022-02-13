#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "tpip.h"
#include "mac.h"
#include "tpip_arp.h"

#define TFTP_PORT 69

enum opcode{RRQ = 1, WRQ, DATA, ACK, ERROR};

enum error_code{NOT_DEFINE, FILE_NOT_FOUND, ACC_VIOLATION, 
                DISK_FULL, ILLEGAL, UNKNOWN_TID, FILE_EXIST, NO_USER};

enum mode{netascii, octet, mail};

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

typedef struct tftp_msg
{
    uint16_t opcode;
    char * filename;
    uint16_t blocknum;

}tftp_msg;

// *********************** Build up the basic 5 TFTP responses *********************
// Opcode 01/02, RRQ/WRQ require
int tpip_tftp_buildrwreq(unsigned char* udp_data, unsigned char op, unsigned char* filename, unsigned char* mode);
// OPcode 03, DATA, transmission ends when it is less than 512 bytes
int tpip_tftp_builddata(unsigned char* udp_data, int blocknum, char* data);
// Opcode 04, ACK, has the same block number as the previous DATA block number
void tpip_tftp_buildack(unsigned char* udp_data, int blocknum);
// Opcode 05, ERROR 
int tpip_tftp_builderror(unsigned char* udp_data, int errorcode, char* errormsg);

// *********************** TFTP receive packet operations *******************************
// Get information from received packets
// Operation code
unsigned short tpip_tftp_get_op(unsigned char* udp_data);
// Block number
unsigned short tpip_tftp_get_blocnum(unsigned char* udp_data);
// File name
int tpip_tftp_get_filename(unsigned char* udp_data, char* filename);
// Mode
int tpip_tftp_get_mode(unsigned char* udp_data, char* mode);
// DATA inside the packet
int tpip_tftp_get_data(unsigned char* udp_data, char* data_buffer);
// Error code
unsigned short tpip_tftp_get_errcode(unsigned char* udp_data);
// Error Message
int tpip_tftp_get_errmsg(unsigned char* udp_data, char* errormsg);

// *********************** TFTP (receive/send data)/(reply ack) functions ********************
// Build up reply
int tpip_tftp_process_reply_ack(ip_t* ip_recv,unsigned short srcport,unsigned char *bufin, unsigned char *bufout, unsigned int tx_addr);
// 2 Modes: netascii and octet, the previous one sends ascii code, the second one is binary  
int tpip_tftp_send_rrq(unsigned char op, unsigned char* filename, unsigned char* mode,unsigned char* bufout, unsigned int tx_addr);
 