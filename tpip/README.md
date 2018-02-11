# Real-Time HTTP/TCP/IP and UDP/IP Stack for IoT (ISORC)

This project is on building time-predictable networked devices.
As a starting point we look how to implement a time-predictable TCP/IP stack.
First experiments have been done with Scala.
Currently we focus on implementing the tpIP stack in C to be worst-case execution
time (WCET) analyzable and to exeute on the Patmos/T-CREST platform.

We plan to submit a first paper in this project to ISORC 2018.

As a next step we will look into REST for IoT on top if the tpIP stack.

Instructions:
First, the host pc must listen:
```
make tpiphost
```

and then run patmos

```
make tpippatmos
```

A flag is set by patos, which the host pc reacts to, and clears, and sets a new acknowledge flag.
