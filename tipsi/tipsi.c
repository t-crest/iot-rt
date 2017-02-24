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
          = {token_rt : 0xedda,
             sprt_rt  : 0xedda,
             dprt_rt  : 0xedda,
             seqn_rt  : 0xedda,
             ackn_rt  : 0xedda};

int sender_send(tcppacketrt_t tcppacketrt){
  receiver_receive (tcppacketrt);    
  return 1;
}

int receiver_receive (tcppacketrt_t tcppacketrt){
  int maxrtdelay = tcppacketrt.seqn / tcprt.token_rt;
  
  tcppacketrt_t tcppacketrt_recv 
                  = {sprt : 0xedda,
                     dprt : 0xedda,
                     syn  : 1,
                     ack  : 1}; 

  if (maxrtdelay <= 10) // ok with 10 ms
    tcppacketrt_recv.win = maxrtdelay;
  else // cannot meet this requirement
    tcppacketrt_recv.win = 0;

  receiver_send(tcppacketrt_recv);
  
  return 1;
}

int receiver_send(tcppacketrt_t tcppacketrt){
    sender_receive(tcppacketrt);
    return 1;
}

int sender_receive(tcppacketrt_t tcppacketrt){
    if (tcppacketrt.win == 10) // receiver ok with 10 ms
      printf("Receiver ok with 10 ms max delay");
    else
      printf("Receiver cannot meet request for max delay\n");
    
    return 1;
}

int main (void){
  printf ("Tipsi, the world's smallest real-time packet simulator!\n");
  
  // set it up with max 10 ms delay
  tcppacketrt_t tcppacketrt_send 
                  = {sprt : 0xedda,
                     dprt : 0xedda,
                     seqn : 10 * 0xedda,
                     syn  : 1};
                     
  // ask receiver                  
  sender_send(tcppacketrt_send);
  
  // send data 
  // ...
  
  // wait max 10 ms for data 
  // ...
  

  return 0;
}