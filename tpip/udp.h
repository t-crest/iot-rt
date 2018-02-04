// udp stuff..

// typedef struct udpstruct_t
// {
//   ipstruct_t *ipstructp;
//   unsigned char *header; // also the ip "data"
//   unsigned char *data;   // this is the udp data
// } udpstruct_t;

#ifndef UDP_H
#define UDP_H

struct udp_msg
{
    short srcport;
    short dstport;
    // bytes
    short length; // update this (and IP total length) each time data is changed
    short checksum;
    char *data;
};

#endif // UDP_H