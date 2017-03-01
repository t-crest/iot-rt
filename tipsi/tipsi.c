// Copyright: 2017, CBS/DTU
// Authors: Rasmus Ulslev Pedersen (rup.itm@cbs.dk)
//          Martin Schoeberl (masha@dtu.dk)
// License: Simplified BSD License
//
// tipsi: simulator for tcp etc. packets

#include <stdio.h>
#include "tipsi.h"

// shared knowledge by sender and receiver
tcprt_t tcprt 
          = {token_rt : 0x1000, // token is the destination port
             dprt_rt  : 0x1000, // 10 * 0x1000 = 0xA000
             sprt_rt  : 0xA000             
            };

// 1
int sender_send(tcppacketrt_t tcppacketrt){
  receiver_receive (tcppacketrt);    
  return 1;
}

// 2
int receiver_receive (tcppacketrt_t tcppacketrt){
  int maxrtdelay = tcppacketrt.seqn / tcprt.token_rt;
  // send to sender
  tcppacketrt_t tcppacketrt_recv;
  int ok = 1; //:-), real time is easy
  if (ok == 1){  
    tcppacketrt_recv.dprt = 0xA000;
    tcppacketrt_recv.sprt = 0xA000;
    tcppacketrt_recv.syn  = 1;
    tcppacketrt_recv.ack  = 1; 
  }
  else {
    tcppacketrt_recv.dprt = 0xA000;
    tcppacketrt_recv.sprt = 0xA000 + 1; // to say that deadline cannot be met  
    tcppacketrt_recv.syn  = 1;
    tcppacketrt_recv.ack  = 1; 
  }
  receiver_send(tcppacketrt_recv);
  return 1;
}

// 3
int receiver_send(tcppacketrt_t tcppacketrt){
    sender_receive(tcppacketrt);
    return 1;
}


// 4
int sender_receive(tcppacketrt_t tcppacketrt){
    int res;
    if (tcppacketrt.sprt == 10 * tcprt.token_rt) {// receiver ok with 10 ms
      printf("Receiver ok with 10 ms max delay");
      res = 1;
    }
    else {
      printf("Receiver cannot meet request for max delay\n");
      res = -1;
    }
    return res;
}

int main (void){
  printf ("Tipsi, the world's smallest real-time packet simulator!\n");
  int max_delay = 10;
  // set it up with max 10 ms delay
  tcppacketrt_t tcppacketrt_send 
                  = {sprt : 0xedda,
                     dprt : 0xedda,
                     seqn : max_delay * tcprt.token_rt,
                     syn  : 1};
                     
  // ask receiver                  
  sender_send(tcppacketrt_send);
  
  // send data 
  // ...
  
  // wait max 10 ms for data 
  // ...
  

  return 0;
}