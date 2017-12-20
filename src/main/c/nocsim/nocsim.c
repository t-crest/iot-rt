/*
  nocsim, one-way-mem noc simulator

  Copyright: CBS, DTU
  Authors: Rasmus Ulslev Pedersen, Martin Schoeberl
  License: Simplified BSD
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define flit unsigned long

// configuration
#define NUM_CORES 4
#define RX_MEM 1
#define TX_MEM 1

// one-way-mem
typedef struct OneWayMemory
{
  unsigned long** data;
} OneWayMemory;

// receiver memory
typedef struct RXMemory
{
  unsigned long** data; // see RX_MEM define
} RXMemory;

// transmission memory
typedef struct TXMemory
{
  unsigned long** data // see TX_MEM define
} TXMemory;

typedef struct Link
{
  Link* al; // another link
} Link;

// network interface
typedef struct NetworkInterface
{
  Link* l; // local 
} NetworkInterface;


typedef struct Router
{ // clockwise "enumeration" when indexed using 'link' below
  Link* l; // local
  Link* n; // north
  Link* e; // east
  Link* s; // south
  Link* w; // west
  Link** link; // allow indexing into l, n, e, s, w
} Router;

typedef struct Core
{
  // some function to be run by the core
  int (*run)(int)
} Core;

// 
typedef struct Tile
{
  Core* core;
  RXMemory* rxmem;
  TXMemory* txmem;
  NetworkInterface* ni;
  Router* router;
} Tile;

// The network-on-chip / network-of-cores structure
typedef struct NoC
{
  Tile** tile;  
} NoC;

NoC* initnoc(){

  
}

void *
corerun(void *coreid)
{
  long cid;
  cid = (long)coreid;
  printf("NoC: core #%ld here ...\n", cid);
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  pthread_t threads[NUM_CORES];
  for (long i = 0; i < NUM_CORES; i++)
  {
    printf("Init nocsim: core %ld created ...\n", i);
    int returncode = pthread_create(&threads[i], NULL, corerun, (void *)i);
    if (returncode)
    {
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

// https://llvm.org/docs/CodingStandards.html