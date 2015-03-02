#include <stdio.h>
#include <mpi.h>
#include <math.h>

int head(double* x, double *y, double *z) { 
  MPI_Request r; 
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  MPI_Bcast(x,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  MPI_Reduce(x,y,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(x,z,1,MPI_DOUBLE,MPI_PROD,0,MPI_COMM_WORLD);
  if (world_rank==0) { 
    *y = *y*3;
    *z = *z*3;
  }
  return 0;
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y,z;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y,&z);
    printf(__FILE__ ": process 0 got  (%f,%f) \n", y,z);
  } else {
    head(&x,&y,&z);
  } 
  MPI_Finalize();
  return 0;
}
