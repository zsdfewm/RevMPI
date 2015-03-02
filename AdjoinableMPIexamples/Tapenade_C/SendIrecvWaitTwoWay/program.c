/* Simple Irecv+Send+Wait || Irecv+Wait+Send */

#include <mpi.h>
#include <stdio.h>
#include <math.h>

int head(double* x, double *y) { 
  MPI_Request r; 
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    MPI_Irecv(y, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD,&r);
    *x=*x*2;
    MPI_Send(x, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
    MPI_Wait(&r,MPI_STATUS_IGNORE);
    *y=*y*3;
  } else if (world_rank == 1) {
    double local;
    MPI_Irecv(&local, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,&r);
    MPI_Wait(&r,MPI_STATUS_IGNORE);
    local=sin(local);
    MPI_Send(&local, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  } 
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=2.5;
    printf(__FILE__ ": process %i sends val [ 2.500000==]%f \n", world_rank, x);
    head(&x,&y);
    printf(__FILE__ ": process %i recvs val [-2.876773==]%f \n", world_rank, y);
    y=y+x ;
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
