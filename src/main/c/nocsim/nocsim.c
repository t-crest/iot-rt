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