
# Testing the Second Serial Port

The second port is a USB/TTL cable connected to IO pins on the expansion header.
It is connected to the expansion port at the bottom (see DE2 user manual on page 47).

Connect a USB/TTL serial cable to those pins:

```


         3.3V  | * * |  GND
               | * * |
               | * * |
               | * * |
               | * * |
    RX -- AH23 | * * |  AG26 -- TX
               +-----+
```

RX and TX are from Patmos view. Connect TX from your device to RX of Patmos,
and RX to TX.

Start a terminal (gktterm) on the USB port of the USB/TTL cable.
This is probably your second port /dev/ttyUSB1.
Set to baud rate 115200.


Configure Patmos in the patmos repo (`~/t-crest/patmos`) with `make config`.

Then run the compilation of hello2.c and download it in *this* repository with:

```
make all down
```

You should see some greeting in the terminal window.




