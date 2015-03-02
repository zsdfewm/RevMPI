#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head(double* x, double *y) { 
  int world_rank;
  double t; 
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  *x = sin(*x*(world_rank+1));
  AMPI_Allreduce(x,&t,1,TYPE_MACRO,MPI_MAX,MPI_COMM_WORLD);
  *x=sin(*x*t);
  AMPI_Reduce(x,y,1,TYPE_MACRO,MPI_SUM,0,MPI_COMM_WORLD);
  if (world_rank==0) *y *= 3;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f \n", y);
  } 
  else {
    head(&x,&y);
  } 
  AMPI_Finalize_NT();
  return 0;
}
