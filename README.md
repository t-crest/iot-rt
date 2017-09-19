# iot-rt
Real-Time HTTP/TCP/IP and UDP/IP Stack for IoT

The tipsi directory has a small simulator for testing out ideas on how to use the IP protocols for setting up messages with a specified max. delay.

## First Experiments

Send IP packets on top of TCP or HTTP post. Our packet is 2 bytes length plus data. For raw TCP we use port 99, for HTTP port 80. In the HTTP post request we encode the binary packet in hexadecimal. After each TCP send the TCP connection is closed.

We will use https://ngrok.com for tunneling.
