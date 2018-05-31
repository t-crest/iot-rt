#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "slip.h"
#include "tpip.h"

// TODO: define constant
static unsigned char bufin[2000];
static unsigned char bufout[2000];

// show how to send an ip packet over a network,
// receive it, and look in the UDP data
int main()
{
    memset(bufin, 0, sizeof(bufin));
    memset(bufout, 0, sizeof(bufout));

    if(!initserial())
      printf("error: serial port not initialized\n");

    int clearcount = serialclear();
    printf("pc step 0: clear serial port for old data: %d bytes\n", clearcount);

    printf("pc step 1: now waiting for an UDP packet from patmos with (without) a flag set in the obb test\n");
    int count = serialreceive(bufin, sizeof(bufin));

    printf("bufin, %d bytes:\n", count);
    bufprint(bufin, count);
    printf("\n");   

    ip_t *ipin = malloc(sizeof(ip_t));
    ipin->udp.data = (char[]){0, 0, 0, 0};
    //ip_t *ipin = (ip_t*)bufin;
    unpackip(ipin, bufin);

    printf("ipin:\n");
    printf("BUG: the slip delimiter is in byte[0]\n");
    printipdatagram(ipin);
    printf("\n");   

    printf("pc step 2: the pc will now clear the obb flag and set another one as a response\n");
    obb_t obb_msg_ack;
    obb_t *obb_msg = (obb_t *) ipin->udp.data;
    if(obb_msg->flags)
      obb_msg_ack.flags = 1;
    else
      obb_msg_ack.flags = 0;

    // prepare sending
    //   patmos, 10.0.0.2, 10002
    //   server, 10.0.0.3, 10003
    ip_t ipoutack = {.verhdl = (0x04 << 4) | 0x05,
                     .tos = 0x00,
                     .length = 20 + 8 + 4, // 5 + 2 + 1 words
                     .id = 1,
                     .ff = 0x4000,
                     .ttl = 0x40,
                     .prot = 0x11, // UDP
                     .checksum = 0x0000,
                     .srcip = (10 << 24) | (0 << 16) | (0 << 8) | 3,
                     .dstip = (10 << 24) | (0 << 16) | (0 << 8) | 2,
                     .udp.srcport = 10003, // 0x2713
                     .udp.dstport = 10002, // 0x2712
                     .udp.length = 8 + 4,
                     .udp.data = (unsigned char[]){obb_msg_ack.flags, 0, 0, 0}};

    int len = packip(bufout, &ipoutack);
    printf("bufout, %d bytes\n", len);
    bufprint(bufout, len);

    int sent = serialsend(bufout, len);

    printf("obb flag test completed on host...sent %d bytes\n", sent);

    return 0;
}








































































// // Basic testing
// int main()
// {
//   printf("Hello tpip world! \n\n");
//   if (!tests())
//     printf("testing: one or more unit tests failed\n\n");
//   else
//     printf("testing: all tests ok\n\n");

//   // temp ip test
//   //test_initip();
//   testwcet();
//   //int foobar = bar + bar;
//   //printf("\n", foobar);

//   // 4500 0073 0000 4000 4011 *b861* c0a8 0001 c0a8 00c7
//   // gives 0x0000 if 0xb861 is added in
//   //int sum                   = 0x4500 + 0x0073 + 0x0000 + 0x4000 + 0x4011 +
//   //                            0xc0a8 + 0x0001 + //0xB861 +
//   //                            0xc0a8 + 0x00c7;
//   //printf("sum               = 0x%08X\n", sum);
//   //int carry                 = (sum & 0xFFFF0000) >> 16;
//   //printf("carry             = 0x%08X\n", carry);
//   //int sum16                 = sum & 0x0000FFFF;
//   //printf("sum16             = 0x%08X\n", sum16);
//   //int sum16pluscarry        = sum16 + carry;
//   //printf("sum16pluscarry    = 0x%08X\n", sum16pluscarry);
//   //int checksum              = (~sum16pluscarry) & 0x0000FFFF;
//   //printf("checksum '0xb861' = 0x%08X\n", checksum);

//   // mod call that only works with -O0 and not with -O2
//   // when running make wcet
//   //printf("mod(3,2)=%d", mod(3,2));

//   //unsigned char a = 0x80;
//   //unsigned char b = 0x01;
//   //int aplusb = a + b;
//   //printf("m1)sum=0x%x\n",aplusb);

//   // dump to file
//   unsigned char pcapdata[2048];
//   unsigned char *pcapdataptr = pcapdata;

//   // global header
//   //typedef struct pcap_hdr_s {
//   //  guint32 magic_number;   /* magic number */
//   //  guint16 version_major;  /* major version number */
//   //  guint16 version_minor;  /* minor version number */
//   //  gint32  thiszone;       /* GMT to local correction */
//   //  guint32 sigfigs;        /* accuracy of timestamps */
//   //  guint32 snaplen;        /* max length of captured packets, in octets */
//   //  guint32 network;        /* data link type */
//   //} pcap_hdr_t;

//   unsigned int magic_number = 0xa1b2c3d4;
//   memcpy(pcapdataptr, &magic_number, sizeof(magic_number));
//   pcapdataptr += sizeof(magic_number);
//   unsigned short version_major = 0x2;
//   memcpy(pcapdataptr, &version_major, sizeof(version_major));
//   pcapdataptr += sizeof(version_major);
//   unsigned short version_minor = 0x4;
//   memcpy(pcapdataptr, &version_minor, sizeof(version_minor));
//   pcapdataptr += sizeof(version_minor);
//   int thiszone = -3600;
//   memcpy(pcapdataptr, &thiszone, sizeof(thiszone));
//   pcapdataptr += sizeof(thiszone);
//   unsigned int sigfigs = 0;
//   memcpy(pcapdataptr, &sigfigs, sizeof(sigfigs));
//   pcapdataptr += sizeof(sigfigs);
//   unsigned int snaplen = 65535;
//   memcpy(pcapdataptr, &snaplen, sizeof(snaplen));
//   pcapdataptr += sizeof(snaplen);
//   unsigned int network = 1;
//   memcpy(pcapdataptr, &network, sizeof(network));
//   pcapdataptr += sizeof(network);

//   // packet header
//   //typedef struct pcaprec_hdr_s {
//   //  guint32 ts_sec;         /* timestamp seconds */
//   //  guint32 ts_usec;        /* timestamp microseconds */
//   //  guint32 incl_len;       /* number of octets of packet saved in file */
//   //  guint32 orig_len;       /* actual length of packet */
//   //} pcaprec_hdr_t;
//   unsigned int ts_sec = time(NULL);
//   memcpy(pcapdataptr, &ts_sec, sizeof(ts_sec));
//   pcapdataptr += sizeof(ts_sec);
//   unsigned int ts_usec = 0;
//   memcpy(pcapdataptr, &ts_usec, sizeof(ts_usec));
//   pcapdataptr += sizeof(ts_usec);
//   unsigned int incl_len = 14 + 20;
//   memcpy(pcapdataptr, &incl_len, sizeof(incl_len));
//   pcapdataptr += sizeof(incl_len);
//   unsigned int orig_len = incl_len;
//   memcpy(pcapdataptr, &orig_len, sizeof(orig_len));
//   pcapdataptr += sizeof(orig_len);

//   // ethernet
//   long long unsigned int dstethaddr = 0x010203040506;
//   memcpy(pcapdataptr, &dstethaddr, sizeof(dstethaddr));
//   pcapdataptr += sizeof(dstethaddr);
//   long long unsigned int srcethaddr = 0x020304050607;
//   memcpy(pcapdataptr, &srcethaddr, sizeof(srcethaddr));
//   pcapdataptr += sizeof(srcethaddr);
//   unsigned short etype = 0x0800; //0x0800 IPv4
//   memcpy(pcapdataptr, &etype, sizeof(etype));
//   pcapdataptr += sizeof(etype);

//   //IP v4
//   // RFC 791:  Internet Header Format
//   //  0                   1                   2                   3
//   //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // |Version|  IHL  |Type of Service|          Total Length         |4500 0073    0.. 3
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // |         Identification        |Flags|      Fragment Offset    |0000 4000    4.. 7
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // |  Time to Live |    Protocol   |         Header Checksum       |4011 *b861*  8..11 (*10..11*)
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // |                       Source Address                          |c0a8 0001   12..16
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // |                    Destination Address                        |c0a8 00c7   16..19
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // |                    Options                    |    Padding    |
//   // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//   //initip(&ip[0],
//   unsigned char verihl = 0x45;
//   memcpy(pcapdataptr, &verihl, sizeof(verihl));
//   pcapdataptr += sizeof(verihl);
//   unsigned char tos = 0x00;
//   memcpy(pcapdataptr, &tos, sizeof(tos));
//   pcapdataptr += sizeof(tos);
//   unsigned short tlen = 20;
//   memcpy(pcapdataptr, &tlen, sizeof(tlen));
//   pcapdataptr += sizeof(tlen);

//   unsigned short id = 0x0001;
//   memcpy(pcapdataptr, &id, sizeof(id));
//   pcapdataptr += sizeof(id);
//   unsigned short ff = 0x0000; // flags and fragment offset
//   memcpy(pcapdataptr, &ff, sizeof(ff));
//   pcapdataptr += sizeof(ff);

//   unsigned char ttl = 10;
//   memcpy(pcapdataptr, &ttl, sizeof(ttl));
//   pcapdataptr += sizeof(ttl);
//   unsigned char prot = 0x11; // prot, icmp:1, udp:0x11
//   memcpy(pcapdataptr, &prot, sizeof(prot));
//   pcapdataptr += sizeof(prot);
//   unsigned short hcksum = 0x0000;
//   memcpy(pcapdataptr, &hcksum, sizeof(hcksum));
//   pcapdataptr += sizeof(hcksum);

//   unsigned int srcip = 0x01020304; // srcip
//   memcpy(pcapdataptr, &srcip, sizeof(srcip));
//   pcapdataptr += sizeof(srcip);

//   unsigned int dstip = 0x01020305; // dstip
//   memcpy(pcapdataptr, &dstip, sizeof(dstip));
//   pcapdataptr += sizeof(dstip);

//   int size = pcapdataptr - pcapdata;
//   printf("m0) size=%d", size);

//   FILE *datafile = fopen("tpip.pcap", "wb");
//   fwrite(pcapdata, size, 1, datafile);
//   fclose(datafile);
// }

// // TIMER TESTS //

// int timer_test()
// {
//   int res = 1; // set test to "ok" to begin with
//   // to see more, then set show to 1
//   int show = 0;
//   if (show)
//     printf("timer_test!\n");
//   if (show)
//     printf("PERIOD: %d ms \n\n", PERIOD);
//   //start timer
//   initwaitfornextperiod();
//   int before = currenttimemillis();
//   if (show)
//     printf("Timer before 'waitforoneperiod()':  %10d ms \n", before);
//   int elapsed1 = waitfornextperiod();
//   int after = currenttimemillis();
//   if (show)
//     printf("Timer after 'waitforoneperiod()':   %10d ms \n", after);
//   if (show)
//     printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed1);
//   if (show)
//     printf("Elapsed time since this wfnp call:  %10d ms \n\n", (after - before));
//   // uncomment 'for'  loop to get a false test
//   //for(volatile int i = 0; i < 100000000; i++); // 'volatile' to avoid '-O2' optimization
//   wait(123); // 'wait(321)' will give a false test
//   if (show)
//     printf("Timer before 'waitforoneperiod()':  %10d ms \n", currenttimemillis());
//   int elapsed2 = waitfornextperiod();
//   // test result fail?
//   if (elapsed2 != 123)
//     res = 0;
//   if (show)
//     printf("Timer after 'waitforoneperiod()':   %10d ms \n", currenttimemillis());
//   if (show)
//     printf("Elapsed time since prev. wfnp call: %10d ms \n", elapsed2);
//   //1 ms left
//   wait(999);
//   if (show)
//     printf("Deadline (-1, so hurry up!):        %10d ms \n", deadline());
//   //broke deadline
//   waitfornextperiod();
//   // disable if you want to skip the wait for 1 sec
//   // wait(1001);
//   if (deadline())
//     if (show)
//       printf("Deadline (+1, bye bye...):        %10d ms \n", deadline());

//   if (show)
//     printf("\ntimer_test result: %s\n", res ? "true" : "false");
//   return res;
// }

// // CHECKSUM TESTS //

// int checksum_test()
// {
//   int res = 1; //ok

//   // Test 1: 0xb861 is the checksum
//   //   0..3|Version|  IHL  |Type of Service|          Total Length         |4500 0073
//   //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   //   4..7|         Identification        |Flags|      Fragment Offset    |0000 4000
//   //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   //  8..11|  Time to Live |    Protocol   |         Header Checksum       |4011 *b861*
//   //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // 12..16|                       Source Address                          |c0a8 0001
//   //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   // 16..19|                    Destination Address                        |c0a8 00c7
//   unsigned char ipdata[] = {0x45, 0x00, 0x00, 0x73,
//                             0x00, 0x00, 0x40, 0x00,
//                             0x40, 0x11, 0xFF, 0xFF, // checksum  is 'set' to 0xFFFF
//                             0xc0, 0xa8, 0x00, 0x01,
//                             0xc0, 0xa8, 0x00, 0xc7};
//   ip[0].header = ipdata;
//   int checksum = calculateipchecksum(&ip[0]);
//   //printf("checksum_test=%08x\n", checksum);
//   if (checksum != 0xb861)
//     res = 0; //failed test
//   return res;
// }

// int mod_test()
// {
//   int res = 1; //test ok
//   if (mod(10, 3) != 1)
//     res = 0; // test fail
//   return res;
// }

// // IP TESTS //

// int test_initip()
// {
//   int show = 0;
//   int res = 1; //test ok
//   if (show)
//     printf("start of test_initip\n");
//   // ip test
//   ip[0].header = buf[0]; //todo: check
//   initip(&ip[0],
//          4,          // ver
//          5,          // hdrwrds
//          0,          // tos
//          0,          // id
//          0,          // flags
//          0,          // foff
//          10,         // ttl
//          0x11,       // prot, icmp:1, udp:17
//          0x01020304, // srcip
//          0x01020305, // dstip
//                      // int* data
//          20,         // tlen, todo:not correct
//          0xFFFF);    // int hchksum  (0xFFFF will leave it alone)

//   if (!(getiptlen(&ip[0]) == 20))
//   {
//     if (show)
//       printf("Error: getiptlen(ip[0])==%d\n", getiptlen(&ip[0]));
//     res = 0; // test fail
//   }
//   else
//   {
//     if (show)
//       printf("Ok: getiptlen(ip[0])==%d\n", getiptlen(&ip[0]));
//   }

//   // with udp payload
//   udp[0].header = ip[0].data;
// /* link error because of compile error
//   initudp(&udp[0],
//           4321,                           // int srcport,
//           1234,                           // int dstport,
//           8 + 2,                          // int len,
//           0xFFFF,                         // int chksum (will be calculated if called with 0xFFFF)
//           (unsigned char[2]){0x62, 0x62}, // some data
//           2);                             // how much data (in elements, i.e., char(s))

// */

//   if (!(getudplen(&udp[0]) == 10))
//   {
//     if (show)
//       printf("Error: getudplen(udp[0])==%d\n", getudplen(&udp[0]));
//     res = 0; // test fail
//   }
//   else
//   {
//     if (show)
//       printf("Ok: getudplen(udp[0])==%d\n", getudplen(&udp[0]));
//   }
//   return res;
// }

// // WCET "tests": just here to platin can reach the code from main
// //void testwcet() __attribute__((noinline));
// void testwcet()
// {
//   // 'volatile' is important so gcc does not optimize the function call
//   // and then platin can't find it...
//   volatile int foo = -1;
//   foo = getipdstip(&ip[0]);
//   foo = waitfornextperiod();
// }

// final tests //

// // insert new unittests here
// int tests()
// {
//   int show = 1; // set to 1 if intermediate results are desired
//   int res = 1;  //test ok
//   // some tests
//   if (!checksum_test())
//   {
//     if (show)
//       printf("checksum_test() failed\n");
//     res = 0; // test fail
//   }
//   if (!timer_test())
//   {
//     if (show)
//       printf("timer_test() failed\n");
//     res = 0; // test fail
//   }
//   if (!mod_test())
//   {
//     if (show)
//       printf("mod_test() failed\n");
//     res = 0; // test fail
//   }
//   if (!test_initip())
//   {
//     if (show)
//       printf("test_initip() failed\n");
//     res = 0; // test fail
//   }
//   return res;
// }
