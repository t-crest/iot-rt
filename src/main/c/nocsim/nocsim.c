/*
  nocsim, one-way-mem noc simulator

  Copyright: CBS, DTU
  Authors: Rasmus Ulslev Pedersen, Martin Schoeberl
  License: Simplified BSD
*/

// Notes
// Structs are pointer
// Data, such as flits and arrays of unsigned longs are not pointers

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define flit unsigned long

// noc configuration
#define MESH_ROWS 2
#define MESH_COLS 2
#define CORES (MESH_ROWS * MESH_COLS)
// core configuration
#define TX_MEM 1
#define RX_MEM 1 * (CORES - 1) // 3

// transmission one-way-memory memory
typedef struct TXMemory
{
  unsigned long data[TX_MEM];
} TXMemory;

// receiver one-way-memory memory
typedef struct RXMemory
{
  unsigned long data[RX_MEM];
} RXMemory;

typedef struct Link Link;
typedef struct Router Router;
typedef struct Link
{
  Router *r; // owner
  Link *al;  // link to (another router) link
} Link;

// network interface
typedef struct NetworkInterface
{
  Link *l; // local
} NetworkInterface;

typedef struct Router
{
  Link l;      // local
  Link n;      // north
  Link e;      // east
  Link s;      // south
  Link w;      // west
  flit regout; // pipelined with a 'flit' (see 'flit' define)
} Router;

typedef struct Core
{
  // some function to be run by the core
  //int (*run)(int)
} Core;

//
typedef struct Tile
{
  Core core;
  TXMemory txmem;
  RXMemory rxmem;
  NetworkInterface ni;
  Router router;
} Tile;

// The network-on-chip / network-of-cores structure
typedef struct NoC
{
  Tile tile[MESH_ROWS][MESH_COLS];
} NoC;

// init the "network-on-chip" NoC
void initnoc(NoC *nocp)
{
  // noc tiles init
  for (int i = 0; i < MESH_ROWS; i++)
  {
    for (int j = 0; j < MESH_COLS; j++)
    {
      nocp->tile[i][j].txmem.data[0] = i << 0x10 | j;       // "i,j" tx test data ...
      nocp->tile[i][j].ni.l = &(nocp->tile[i][j].router.l); // connect ni and router
    }
  }
}

void initrouter(NoC *nocp)
{
  // router links init
  for (int i = 0; i < MESH_ROWS; i++)
  {
    for (int j = 0; j < MESH_COLS; j++) //...
    {
      nocp->tile[i][j].router.n.r = &(nocp->tile[i][j].router);
      nocp->tile[i][j].router.e.r = &(nocp->tile[i][j].router);
      nocp->tile[i][j].router.s.r = &(nocp->tile[i][j].router);
      nocp->tile[i][j].router.w.r = &(nocp->tile[i][j].router);
      nocp->tile[i][j].router.l.r = &(nocp->tile[i][j].router);
      // torus property by connecting 'e' to 'w'
      int m = (i + 1 < MESH_ROWS - 1) ? i + 1 : 0;
      nocp->tile[i][j].router.e.al = &(nocp->tile[m][j].router.w);
      // torus property by connecting 'n' to 's'
      int n = (j + 1 < MESH_COLS - 1) ? j + 1 : 0;
      nocp->tile[i][j].router.n.al = &(nocp->tile[i][n].router.s);
    }
  }
}

void *corerun(void *coreid)
{
  long cid;
  cid = (long)coreid;
  printf("NoC: core #%ld here ...\n", cid);
  pthread_exit(NULL);
}

static NoC noc;
int main(int argc, char *argv[])
{

  initnoc(&noc);
  initrouter(&noc);

  for (int i = 0; i < MESH_ROWS; i++)
  {
    for (int j = 0; j < MESH_COLS; j++) //...
    {
      printf("router[%d][%d] rx data: 0x%08lx\n", i, j, noc.tile[i][j].txmem.data[0]);
    }
  }

  //routing test
  //e
  noc.tile[0][0].router.e.al->r->regout = 2; // .tile[i][j].txmem[0]
  printf("\nrouter[0][1].regout: 0x%08lx\n", noc.tile[0][0].router.regout);

  //routing test
  //nel
  // nl
  //  el
  //...got this far 

  pthread_t threads[CORES];
  for (long i = 0; i < CORES; i++)
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