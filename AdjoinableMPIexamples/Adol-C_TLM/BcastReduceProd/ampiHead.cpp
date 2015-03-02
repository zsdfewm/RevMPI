#include <cstdio>
#include <cmath>
#include "ampiHead.h"
#include "ampi/ampi.h"

void head(adouble* x, adouble *y) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  AMPI_Reduce(x,y,1,TYPE_MACRO,MPI_PROD,0,MPI_COMM_WORLD);
  if (world_rank==0) *y = *y*3;
}
