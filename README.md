# Real-Time HTTP/TCP/IP and UDP/IP Stack for IoT

This project is on building time-predictable networked devices.
As a starting point we look how to implement a time-predictable TCP/IP stack.
First experiments have been done with Scala.
Currently we focus on implementing the tpIP stack in C to be worst-case
execution time (WCET) analyzable and to execute on the Patmos/T-CREST platform.

We submitted an initial paper on this project to ISORC 2018.
Details on the tpIP stack in C can be found in folder [tpip](tpip).

## Scala Based Experiments

Initial experiments in Scala can be started with following make targets:

Start ngrok for the tunnel with:

```
make ngrok
```

And in a different terminal start the example application with:

```
make app
```

## C Based Experiements

see folder [tpip](tpip).

## Next Steps

Implement the UDP protocol from the railway application.

As a next step we will look into REST for IoT on top if the tpIP stack.

At the link layer we plan to also use TTEthernet.
