# tpIP a Time-predictable TCP/IP Stack

This folder contains the source code supporting following paper:

Martin Schoeberl and Rasmus Ulslev Pedersen.
tpIP: a Time-predictable TCP/IP Stack for Cyber-Physical Systems.
[*Submitted to ISORC 2018*](https://cps-research-group.github.io/ISORC2018/html/program.html).

Most of our evaluation is using the T-CREST/Patmos processor in an FPGA.
For the general build instructions of T-CREST please look into the
[Main README of Patmos](https://github.com/t-crest/patmos).

```
**************************
Application *  Blaahund,TFTP
**************************
Transport   *  tpip	
Network     *  tpip
**************************
Datalink    * Slip, Ethernet
**************************
```
# A Second Serial Port

We use SLIP as link layer protocol, which means the PC and the FPGA board
need a second serial port.

The second port is a USB/TTL cable connected to IO pins on the expansion header.
It is connected to the expansion port at the bottom (see Figure 4-15
GPIO Pin Arrangement in the DE2 user manual on page 46).

To build a Patmos configuration with the second port as the project `altde2-all`.
This is best setup by using a local `config.mk` file (in `t-crest/patmos`) as follows:

```
BOARD=altde2-all
```

Connect the USB/TTL serial cable to the following pins on GP10:

```
               | * * |  GND
               | * * |
               | * * |
               | * * |
               | * * |
 RX -- 39/AH23 | * * |  40/AG26 -- TX
               +-----+
                GP 10
```

RX and TX names are from Patmos view. Connect TX from your USB/TTL
device to RX of Patmos, and RX to TX.

For information how to test this serial setup see at [patmos](../patmos).

Questions to: martin@jopdesign.com


# tpIP Experiments

The following experiment is a reimplementation of a UDP based protocol
used in a railway application on single track lines of the Austrian
railway (OEBB).


First, the host pc must listen:
```
make tpiphost
```

and then run patmos

```
make tpippatmos
```

A flag is set by Patmos, which the host pc reacts to, and clears, and sets a new acknowledge flag.

Note: If `make tpiphost` results in an error, then try to run tpippatos once to ensure the serial port is enabled.

# Using Ethernet as the link layer
## For Linux Virtual Machine:

If you are using a Virtual machine like VMware Workstation, first configure the network adaptor setting as 'custom'
or bridge mode, I set it as custom. Then choose a virtual network, probably VMnet0. Next, go to the virtual network 
editor and choose the correct Network Interface Card(NIC) for the virtual network which should be used as a cable 
connected Ethernet controller. In some linux version, you can set the IP address directly from a GUI, while I couldn't.
Solution:(Ubuntu 20.02)

cd /etc/netplan/
There should be a file called '\*.yaml', in my case it is '*01-network-manager-all.yaml*'
Open it and add the following lines:
```
 ens33:
            dhcp4: no
            addresses: [192.168.24.5/24] #/num is the number of '1' in netmask
            gateway4: 192.168.24.1
            nameservers:
                addresses: [114.114.114.114,8.8.8.8]

```
Now the VM should be able to communicate with the board.

## For Windows:

The IP address of patmos board is 192.168.24.50. Set the relative Windows NIC IP in the same domain, like 192.169.24.10. 

## Monitor
Wireshark works as a monitor to capture UDP packets, [netassist](https://github.com/nicedayzhu/netAssist) is used to send
UDP packets through Ethernet.

## Compilation
Currently, it is only compiled successfully under patmos/c/ dir. 

## Note
Basic MAC controller(eth\_\*, mac.\*) is kept unchanged.
*ICMP, ARP, IPv4, TCP, UDP* are merged into *tpip*, while changes in *ARP* is writen in *tpip*\_*arp.*\*.
No need to use *tte* and *ptp1588*. 

The test demo is in *demo.c* temporaly. 
