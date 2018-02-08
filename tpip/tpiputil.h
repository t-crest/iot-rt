// TPIP util

#ifndef TPIPUTIL_H
#define TPIPUTIL_H

#include <time.h> // CLOCKS_PER_SEC
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tpip.h"

// period length (ms). Can also be cycles if on 'bare metal'
#define PERIOD 100 // 1000 // must be less than 2,147,483,647

// used by 'int currenttimemillis()'
#define CLOCKS_PER_MSEC (CLOCKS_PER_SEC / 1000)

// call this when wanting to read from the serial port on the PC
int listentoserial(unsigned char *bufin);
int listentoserialslip(unsigned char *bufin);


#endif //TPIPUTIL_H