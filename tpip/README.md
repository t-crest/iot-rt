# tpIP a Time-predictable TCP/IP Stack

This folder contains the source code supporting following paper:

Martin Schoeberl and Rasmus Ulslev Pedersen.
tpIP: a Time-predictable TCP/IP Stack for Cyber-Physical Systems.
*Submitted to ISORC 2018*.

Most of our evaluation is using the T-CREST/Patmos processor in an FPGA.
For the general build instructions of T-CREST please look into the
[Main README](../../../README.md).

# A Second Serial Port

We use SLIP as link layer protocol, which means the PC and the FPGA board
need a second serial port.

The second port is a USB/TTL cable connected to IO pins on the expansion header.
It is connected to the expansion port at the bottom (see Figure 4-15
GPIO Pin Arrangement in the DE2 user manual on page 46).

To build a Patmos configuration with the second port us the project `altde2-all`.
This is best setup by using a local `config.mk` file (in `t-crest/patmos`) as follows:

```
BOARD=altde2-all
```

Connect the USB/TTL serial cable to those pins:

```


               | * * |  GND
               | * * |
               | * * |
               | * * |
               | * * |
    RX -- AH23 | * * |  AG26 -- TX
               +-----+
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
