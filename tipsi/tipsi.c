// Copyright: 2017, CBS/DTU
// Authors: Rasmus Ulslev Pedersen (rup.itm@cbs.dk)
//          Martin Schoeberl (masha@dtu.dk)
// License: Simplified BSD License
//
// tipsi: simulator for tcp etc. packets

#include <stdio.h>

int receive (char pload_[], int size)
{
  printf ("%d bytes received\n", size);
  for (int i = 0; i < size; i++){
      printf("0x%X ", pload_[i]);
  }
  printf("\n");
  return 1;
}

int main (void)
{
  printf ("Tipsi, the world's smallest packet simulator!\n");
  char pload[] = { 0xE, 0xD, 0xD, 0xA };
  if(receive (pload, sizeof(pload)) == 1)
      printf("Success!");
  return 0;
}