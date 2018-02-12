# Real-Time HTTP/TCP/IP and UDP/IP Stack for IoT

This project is on building time-predictable networked devices.
As a starting point we look how to implement a time-predictable TCP/IP stack.
First experiments have been done with Scala.
Currently we focus on implementing the tpIP stack in C to be worst-case
execution time (WCET) analyzable and to execute on the Patmos/T-CREST platform.

We plan to submit a first paper in this project to ISORC 2018.
Details on the tpIP stack can be found in folder [tpip](tpip).

As a next step we will look into REST for IoT on top if the tpIP stack.

At the link layer we also plan to use TTEthernet.
