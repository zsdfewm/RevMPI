#include <stdio.h>
#include <mpi.h>
#include <math.h>

void head(double* x, double *y) { 
  int world_rank;
  double t; 
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  MPI_Bcast(x,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  MPI_Allreduce(x,&t,1,MPI_DOUBLE,MPI_PROD,MPI_COMM_WORLD);
  *x=sin(*x*t);
  MPI_Reduce(x,y,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  if (world_rank==0) *y *= 3;
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f \n", y);
  } else {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
