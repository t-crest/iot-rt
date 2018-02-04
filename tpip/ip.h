// ip header

#ifndef IP_H
#define IP_H

#include "udp.h"

// later a union...
struct ip_datagram {
    // bla bla
    struct udp_msg;
};

#endif // IP_H