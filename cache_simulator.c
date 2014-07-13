/*
 * main.c
 *
 *  Created on: 25/12/2013
 *      Author: diego
 */


#include "sim_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main()
{
  unsigned int i, j;
  unsigned long int cs, ls, a, n, dA, dB, dC, ws;

  /*
   * Example of simulation with:
   * Cache size       = 65536 bytes
   * Line size        = 32 bytes
   * Associativity    = 4
   * Nº elements      = 500
   * A matrix address = 1024
   * B matrix address = 10000000
   * C matrix address = 100000000
   * Word size        = 4 bytes (float)
   */

  cs = 65536;
  ls = 64;
  a  = 4;
  n  = 500;
  dA = 1024;
  dB = 10000000;
  dC = 100000000;
  ws = 4;

  /*
   * C = A + B; Row major access
   */
  config_sim(cs, ls, a);

  for(i=0; i<n; i++) {
    for(j=0; j<n; j++) {
      sim_access(dA+ (i * n + j) * ws);
      sim_access(dB+ (i * n + j) * ws);
      sim_access(dC+ (i * n + j) * ws);
    }
  }

  end_sim();

  printf("****************\n");
  printf("Row major access\n");
  printf("****************\n");
  printf("Total access = %d\n", Access());
  printf("Total misses = %d\n", Misses());
  printf("Miss rate = %4.2lf%%\n", (double)Misses()/(double)Access()*100);

  /*
   * C = A + B; Column major access
   */
  config_sim(cs, ls, a);

  for(i=0; i<n; i++) {
     for(j=0; j<n; j++) {
      sim_access(dA+ (j * n + i) * ws);
      sim_access(dB+ (j * n + i) * ws);
      sim_access(dC+ (j * n + i) * ws);
    }
  }

  end_sim();

  printf("\n*******************\n");
  printf("Column major access\n");
  printf("*******************\n");
  printf("Total access = %d\n", Access());
  printf("Total misses = %d\n", Misses());
  printf("Miss rate = %4.2lf%%\n", (double)Misses()/(double)Access()*100);

  return 0;
}


