// minimal test file to get started on obb
// the server receives a request and responds (i.e., acknowledges)
//   with the same request msg id and increments the flag
// more to be done...getting started...

#include <stdio.h>
#include <string.h> //memset
#include <machine/patmos.h>
#include "tpip.h"

// to be expanded as needed
typedef struct obb_t
{
    // flags
    unsigned long flags;
} obb_t;

void printword(unsigned int val)
{
    unsigned int hostorder = ntohl(val);
    printf("0x%02x 0x%02x 0x%02x 0x%02x -> 0x%02x 0x%02x 0x%02x 0x%02x\n", ((val >> 24) & 0xFF), ((val >> 16) & 0xFF), ((val >> 8) & 0xFF), (val & 0xFF),
           ((hostorder >> 24) & 0xFF), ((hostorder >> 16) & 0xFF), ((hostorder >> 8) & 0xFF), (hostorder & 0xFF));
}

static unsigned long bufout[BUFSIZEWORDS];

// this function should be changed to slip
__attribute__((noinline)) void xmitslip(char *pbuf, int len)
{
    volatile _IODEV int *uart2_ptr = (volatile _IODEV int *)0xF00e0004;
    volatile _IODEV int *uart2_status_ptr = (volatile _IODEV int *)0xF00e0000;

    //BUFSIZEWORDS
    _Pragma("loopbound min 0 max 512") for (int i = 0; i < len * 4; ++i)
    {
        // verify worst wait here...
        _Pragma("loopbound min 0 max 1000")
        while (((*(uart2_status_ptr)) & 0x01) == 0)
            ; // busy wait
        *uart2_ptr = *(pbuf + i);
        //printf(" 0x%02x", *(pbuf + i));
    }
}

__attribute__((noinline)) void xmit(char *pbuf, int len)
{
    volatile _IODEV int *uart2_ptr = (volatile _IODEV int *)0xF00e0004;
    volatile _IODEV int *uart2_status_ptr = (volatile _IODEV int *)0xF00e0000;

    //BUFSIZEWORDS
    _Pragma("loopbound min 0 max 512") for (int i = 0; i < len * 4; ++i)
    {
        // verify worst wait here...
        _Pragma("loopbound min 0 max 1000")
        while (((*(uart2_status_ptr)) & 0x01) == 0)
            ; // busy wait
        *uart2_ptr = *(pbuf + i);
        //printf(" 0x%02x", *(pbuf + i));
    }
}

int main(int argc, char *argv[])
{

    memset(bufout, 0, sizeof(bufout));

    obb_t obb_msg = (obb_t){.flags = 1};

    // prepare sending
    //   patmos, 10.0.0.2, 10002
    //   server, 10.0.0.3, 10003
    ip_t ipout = {.verhdl = (0x4 << 4) | 0x5,
                  .tos = 0x00,
                  .length = 20 + 8 + 4, // 5 + 2 + 1 words
                  .id = 1,
                  .ff = 0x4000,
                  .ttl = 0x20,
                  .prot = 0x11, // UDP
                  .checksum = 0x0000,
                  .srcip = (10 << 24) | (0 << 16) | (0 << 8) | 2,
                  .dstip = (10 << 24) | (0 << 16) | (0 << 8) | 3,
                  .udp.srcport = 10002, // 0x2712
                  .udp.dstport = 10003, // 0x2713
                  .udp.length = 8 + 4,
                  .udp.data = (unsigned char[]){0, 0, 0, (unsigned char)obb_msg.flags}};

    //char hello[] = "Hello Second World\r\n";
    int len = packip(bufout, &ipout);

    for (int i = 0; i < 8; i++)
        printword(bufout[i]);

    char *pbuf = (char *)bufout;

    printf("patmos sending: \n");
    xmit(pbuf, len);
    xmitslip(pbuf, len);
    printf("\n");
    printf("obb flag test completed on patmos...\n");

    return 0;
}