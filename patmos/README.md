
# Testing the Second Serial Port

The second port is a USB/TTL cable connected to IO pins on the expansion header.
It is connected to the expansion port at the bottom (see DE2 user manual on page 47).

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

RX and TX names are from Patmos view. Connect TX from your USB/TTL device to RX of Patmos,
and RX to TX.

Start a terminal (gktterm) on the USB port of the USB/TTL cable.
This is probably your second port /dev/ttyUSB1.
Set to baud rate 115200.


Configure Patmos in the patmos repo (`~/t-crest/patmos`) with `make config`.

Then run the compilation of hello2.c and download it in *this* repository with:

```
make all down
```

You should see some greeting in the terminal window coming from the second
serial port.




