// udp stuff..

// typedef struct udpstruct_t
// {
//   ipstruct_t *ipstructp;
//   unsigned char *header; // also the ip "data"
//   unsigned char *data;   // this is the udp data
// } udpstruct_t;

typedef struct udp_datagram
{
    short srcport;
    short dstport;
    // bytes
    short length; // update this each time data is changed
    short checksum;
    char *data;
} udp_datagram;