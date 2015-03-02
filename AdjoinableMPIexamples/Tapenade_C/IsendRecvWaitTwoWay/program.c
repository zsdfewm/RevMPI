/* Simple Isend+Wait+Recv || Recv+Isend+Wait */

#include <mpi.h>
#include <stdio.h>
#include <math.h>

int head(double* x, double *y) { 
  MPI_Request r; 
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    *x=*x*2;
    MPI_Isend(x, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD,&r);
    MPI_Recv(y, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Wait(&r,MPI_STATUS_IGNORE);
    *y=*y*3;
  } else if (world_rank == 1) {
    double local;
    MPI_Recv(&local, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local=sin(local);
    MPI_Isend(&local, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,&r);
    MPI_Wait(&r,MPI_STATUS_IGNORE);
  } 
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    printf(__FILE__ ": process %i sends val [3.500000==]%f\n", world_rank, x);
    head(&x,&y);
    printf(__FILE__ ": process %i recvs val [1.970960==]%f\n", world_rank, y);
    y=y+x ;
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
