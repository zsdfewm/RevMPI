#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head(double* x, double *y) { 
  int world_rank;
  int world_size;
  int i,root=0;
  double t;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  if (world_rank==0) {
    *y=1.0;
  }
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  for (i=1;i<world_size;++i)  { 
    if (i==world_rank){ 
      *x*=i;
      AMPI_Send(x,1,TYPE_MACRO,0,0,AMPI_TO_RECV,MPI_COMM_WORLD);
    }
    if(world_rank==root) { 
      AMPI_Recv(&t,1,TYPE_MACRO,MPI_ANY_SOURCE,MPI_ANY_TAG,AMPI_FROM_SEND,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      *y+=i*t;
    }
    AMPI_Barrier(MPI_COMM_WORLD); /* need this to make it deterministic */
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %e \n", y);
  } 
  else {
    head(&x,&y);
  } 
  AMPI_Finalize_NT();
  return 0;
}
