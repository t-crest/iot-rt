// TPIP util

#ifndef TPIPUTIL_H
#define TPIPUTIL_H

// period length (ms). Can also be cycles if on 'bare metal'
#define PERIOD 100 // 1000 // must be less than 2,147,483,647

// used by 'int currenttimemillis()'
#define CLOCKS_PER_MSEC (CLOCKS_PER_SEC / 1000)

// todo: put the function definitions here when we are ready to use the timer again

#endif //TPIPUTIL_H