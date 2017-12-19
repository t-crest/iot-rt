// Copyright: 2017, CBS/DTU
// Authors: Rasmus Ulslev Pedersen (rup.itm@cbs.dk)
//          Martin Schoeberl (masha@dtu.dk)
// License: Simplified BSD License
//
// tipsi: Simulator for tcp etc. packets

// real time info
typedef struct { 
   unsigned int token_rt : 16;
   unsigned int sprt_rt  : 16;
   unsigned int dprt_rt  : 16;   
   unsigned int seqn_rt  : 32;
   unsigned int ackn_rt  : 32;
} tcprt_t;

// tcp header
typedef struct { 
   unsigned int sprt  : 16; // Source Port
   unsigned int dprt  : 16; // Destination Port  
   unsigned int seqn  : 32; // Sequence Number
   unsigned int ackn  : 32; // Sequence Number
   // ...
   unsigned int syn   :  1;  // SYN
   unsigned int ack   :  1;  // ACK
   unsigned int win   : 16;  // Window
   // ...
} tcppacketrt_t;

int receiver_receive (tcppacketrt_t tcppacketrt);
int sender_send (tcppacketrt_t tcppacketrt);
int sender_receive (tcppacketrt_t tcppacketrt);
int receiver_send (tcppacketrt_t tcppacketrt);

// TCP Header Format (see https://tools.ietf.org/html/rfc793, Figure 3)
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Source Port          |       Destination Port        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        Sequence Number                        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Acknowledgment Number                      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  Data |           |U|A|P|R|S|F|                               |
// | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
// |       |           |G|K|H|T|N|N|                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           Checksum            |         Urgent Pointer        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Options                    |    Padding    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             data                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+