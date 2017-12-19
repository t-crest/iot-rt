TRT1 (Transmission Real Time 1)
-------------------------------
Real Time Handshake with TCP:
The protocol is such that the first packet (SYN=1) is 
sent to a port on the receiver that is agreed to be
the entry point for setting up TRT transmissions. 
The source port field in the first packet carries a 
special meaning (besides being a source port). The 
sequence number in the first packet is a multiple (/)
of the longest delay (the multiple from using the 
source port as modulus to the sequence number) the 
sender must wait for a response. The remainder 
specifies the time unit: 
0:unspecified such as clock cycles
1:ns, 2:ms, 3:s, 4:?, 5:?
If the receiver can meet the required deadline it
reply with an ACK=1, SYN=1 packet using the same
source port number. If it cannot meet the 
requirement, then it simply replies with a packet to 
the destination port (the source port of the initiator),
but with a different source port specified. 
[Possible option: How to tell the intiator *what* it can
meet if it can't meet the first request].
The iniator then uses the last part of the handshake to 
send the real-time request (ACK=1, FIN=1), and the payload
indicates what operation it wants. That can be a get 
(i.e. read) or a set (i.e. write). But the receiver must
send the final packet (FIN=1) before the max. deadline
promised to the initiator. [Option: if the receiver 
does not set the FIN=1 in the actual request (third 
packet overall) then the system can still set it (FIN=1), 
and then the initiator must send a FIN=1 to the system,
which, if it does not receive that packet, will go into
error state as each get/set much be acknowledged.   

 