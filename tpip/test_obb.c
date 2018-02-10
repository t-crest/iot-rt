// minimal test file to get started on obb
// the server receives a request and responds (i.e., acknowledges)
//   with the same request msg id and increments the flag
// more to be done...getting started...

#include <stdio.h>
#include <string.h> //memset
#include <machine/patmos.h>
#include "slip.h"
#include "tpip.h"

static unsigned char bufout[2000];

__attribute__((noinline)) 
int receive(unsigned char *pbuf)
{
    //todo use tpip_slip_is_esc and tpip_slip_was_esc

    printf("patmos receive:\n");
    int cnt = 0;
    unsigned char c;
    do{
        tpip_slip_getchar(&c);
        if (tpip_slip_is_end(c)) break;
	    *pbuf = c;
        printf("pbuf[%02d] <- 0x%02x\n", cnt, c);
        pbuf++;
        cnt++;
    } while(1);
    printf("! END 0x%02x\n", c);
    return cnt;
}

// this function should be changed to slip
// modify this to encode and send over slip
__attribute__((noinline)) 
void xmitslip(unsigned char *pbuf, int cnt)
{
    // cnt is the number of bytes to transmit
    printf("xmit ip datagram:\n");
    bufprint(pbuf, cnt);    
    printf("total %d bytes\n", cnt);

    //BUFSIZEWORDS
    printf("\n");
    printf("slip send: ");
    int j = 0;
    _Pragma("loopbound min 0 max 512") for (int i = 0; i < cnt; ++i, ++j)
    {
        // verify worst wait here...
        tpip_slip_putchar(*(pbuf + j));
        if (j % 4 == 0) 
            printf("\n%04d: ", j);
        printf(" 0x%02x", *(pbuf + j));
    }
    if (j % 4 == 0) 
        printf("\n%04d: ", j);
    
    tpip_slip_put_end();
    
    printf("xmit total %d bytes\n", j + 1);
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
                  .ttl = 0x40,
                  .prot = 0x11, // UDP
                  .checksum = 0x0000,
                  .srcip = (10 << 24) | (0 << 16) | (0 << 8) | 2,
                  .dstip = (10 << 24) | (0 << 16) | (0 << 8) | 3,
                  .udp.srcport = 10002, // 0x2712
                  .udp.dstport = 10003, // 0x2713
                  .udp.length = 8 + 4,
                  .udp.data = (unsigned char[]){0, 0, 0, (unsigned char)obb_msg.flags}};

    
    printf("ipout:\n");
    printipdatagram(&ipout);
    //char hello[] = "Hello Second World\r\n";
    int len = packip(bufout, &ipout);
    printf("\n");
    
    printf("ipout \"raw\" mem (%d bytes):\n", len);
    bufprint(bufout, len); 
    printf("\n");

    printf("patmos sending: \n");
    //xmit((char *)bufout, len);
    // function for slip
    xmitslip(bufout, len);
    printf("\n");
    unsigned char recbuf[2000];
    memset(recbuf, 0, sizeof(recbuf));
    int reccnt = receive(recbuf);

    printf("ipin mem \"raw\" (%d bytes):\n", reccnt);
    bufprint(recbuf, reccnt); 
    printf("\n");

    ip_t *ipin = (ip_t*) recbuf;
    unpackip(ipin, recbuf);
    printf("ipin:\n");
    printipdatagram(ipin);
    printf("obb flag test completed on patmos...\n");

    return 0;
}
