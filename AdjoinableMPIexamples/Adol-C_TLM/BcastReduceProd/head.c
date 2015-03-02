#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include "head.h"

void head(double* x, double *y) { 
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  MPI_Bcast(x,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  MPI_Reduce(x,y,1,MPI_DOUBLE,MPI_PROD,0,MPI_COMM_WORLD);
  if (world_rank==0) *y = *y*3;
}
