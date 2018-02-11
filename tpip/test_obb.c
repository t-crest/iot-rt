// minimal test file to get started on obb
// the server receives a request and responds (i.e., acknowledges)
//   with the same request msg id and increments the flag
// more to be done...getting started...

#include <stdio.h>
#include <string.h> //memset
#include <machine/patmos.h>
#include "slip.h"
#include "tpip.h"

static unsigned char bufin[2000];
static unsigned char bufout[2000];

__attribute__((noinline)) 
int receive(unsigned char *pbuf)
{
    
    int cnt = 0;
    int slipcnt = 0;
    static unsigned char c;
    do{
      while(!tpip_slip_getchar(&c));
      printf("slip %02d = 0x%02x\n", slipcnt, c);
      slipcnt++;
      if (tpip_slip_is_end(c)){
        if(cnt == 0) 
          continue; // slip frames starting with END
        else
          break;
      } else if (tpip_slip_is_esc(c)){
        while(!tpip_slip_getchar(&c));
        printf("slip %02d = 0x%02x\n", slipcnt, c);
        slipcnt++;
        c = tpip_slip_was_esc(c);  
      }
	  *pbuf = c;
      
      pbuf++;
      cnt++;
    } while(1);
    printf("patmos receive: %d data bytes (%d slip bytes):\n", cnt, slipcnt);    
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

    printf("\n");
    printf("slip send:\n");
    int i = 0;
    int j = 0;
    tpip_slip_put_end(); j++;
    
    for (i = 0; i < cnt; i++, j++)
    {
      unsigned char c = *(pbuf + i);
      printf("%02d:data byte 0x%02x\n", i, c);
      if(tpip_slip_is_esc(c)){
        tpip_slip_put_esc();
        tpip_slip_put_esc_esc();
        j++;
      } 
      else if (tpip_slip_is_end(c)){
        tpip_slip_put_esc();
        tpip_slip_put_esc_end();
        j++;
      } 
      else {
        tpip_slip_putchar(c);
      }
    }
    
    tpip_slip_put_end(); j++;
    
    printf("xmit total %d bytes\n", j);
}

int main(int argc, char *argv[])
{
    memset(bufin, 0, sizeof(bufin));
    memset(bufout, 0, sizeof(bufout));

    unsigned char testmem[4];
    memset(testmem, 0, sizeof(testmem));
    printf("0x%02x\n", testmem[2]);
    testmem[2] = 2;
    printf("0x%02x\n", testmem[2]);
    testmem[3] = 3;
    printf("2 0x%02x\n", testmem[2]);
    testmem[3] = 3;
    printf("3 0x%02x\n", testmem[3]);

    printf("patmos step 1: an UDP packet will now be sent to the PC with 4th byte set in the obb flag struct\n");
    obb_t obb_msg = (obb_t){.flags = 1};

    // prepare sending
    //   patmos, 10.0.0.2, 10002
    //   server, 10.0.0.3, 10003
    ip_t ipout = {.verhdl = (0x4 << 4) | 0x5,
                  .tos = 0x03, //0x00,
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
                  .udp.data = (unsigned char[]){0, 0, 0, 1}};//(unsigned char)obb_msg.flags}};

    
    printf("ipout:\n");
    printipdatagram(&ipout);
    int len = packip(bufout, &ipout);
    ipchecksum(bufout);
    bufprint(bufout, len); 
    printf("\n");
    
    printf("patmos sending: \n");
    xmitslip(bufout, len);
    printf("\n");
 
    printf("patmos step 2: an UDP packet will now be received from the PC with 1st byte set in the obb flag struct\n"); 
    unsigned char recbuf[2000];
    memset(recbuf, 0, sizeof(recbuf));
    int reccnt = receive(recbuf);

    printf("ipin mem \"raw\" (%d bytes):\n", reccnt);
    bufprint(recbuf, reccnt); 
    printf("\n");

    ip_t *ipin = (ip_t*) recbuf;
    unpackip(ipin, recbuf);
    printf("ip datagram from pc:\n");
    printipdatagram(ipin);
    printf("obb flag test completed on patmos...\n");

    return 0;
}
