#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head(double* x, double *y, double *z) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  AMPI_Reduce(x,y,1,TYPE_MACRO,MPI_SUM,0,MPI_COMM_WORLD);
  AMPI_Reduce(x,z,1,TYPE_MACRO,MPI_PROD,0,MPI_COMM_WORLD);
  if (world_rank==0) { 
    *y = *y*3;
    *z = *z*3;
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y,z;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y,&z);
    printf(__FILE__ ": process 0 got number (%f,%f) \n", y,z);
  } 
  else {
    head(&x,&y,&z);
  } 
  AMPI_Finalize_NT();
  return 0;
}
