/*
  nocsim, one-way-mem noc simulator

  Copyright: CBS, DTU
  Authors: Rasmus Ulslev Pedersen, Martin Schoeberl
  License: Simplified BSD
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_CORES	4

void *corerun(void *coreid)
{
   long cid;
   cid = (long)coreid;
   printf("NoC: core #%ld here ...\n", cid);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
   pthread_t threads[NUM_CORES];
   for(long i=0;i<NUM_CORES;i++){
     printf("Init nocsim: core %ld created ...\n", i);
     int returncode = pthread_create(&threads[i], NULL, corerun, (void *)i);
     if (returncode){
       printf("Error %d ... \n", returncode);
       exit(-1);
     }
   }

   
   pthread_exit(NULL);
}

// Temp notes for a little design thinking //
// For a simulation harness, we can make a "nocsim" harness that models the main components in
// C structs, but are not meant to be run on the target (patmos). It has structs like
// one-way-mem-noc struct, router structs, links structs, ni struct, tx mem struct, rx mem struct,
// core struct, node (consists of core, rx, tx, ni) struct, tile (consists of node, router,
// and links) struct.
// Clocking: It is perhaps best to think in cycles as each cycle is a mini-transaction. Milli-second
// based clocking is not going to work. It is fairly easy to use this clocking "scheme" in a
// multithreaded simulator as the (nice) mesochronous clock property makes it possible
// to read the current clock cycle, but care has to be taken to remember that another
// router is perhaps already shifted into the next clock cycle. 
// Ideas for hard simulations, hard calculations:
//   1) Each core wants to read on each cycle (we must be able to *calculate* WCET here as the
//        sum of routing, latency, TDM scheduling *and* local app code wcet (platin analysis of a
//        given benchmark C code).
//   2) Each core wants to to write on each cycle (same argument as above, we should be able to
//        calculate it. 