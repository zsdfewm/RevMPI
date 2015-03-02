/* Several Send's and a loop of wildcard Recv's */

#include <mpi.h>

#include <stdio.h>
#include <math.h>

void head(double* x, double *y) {
  int rank, nbProc, i;
  double recvx, recvy ;
  MPI_Status status ;
  MPI_Request req ;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nbProc) ;
  if (rank==0) {
    for (i=1 ; i<nbProc ; ++i) {
      MPI_Recv(&recvy,1,MPI_DOUBLE, MPI_ANY_SOURCE,12, MPI_COMM_WORLD, &status) ;
      *y = *y*recvy ;
      MPI_Recv(&recvx,1,MPI_DOUBLE, MPI_ANY_SOURCE,11, MPI_COMM_WORLD, &status) ;
      *x = *x+recvx ;
    }
  } else {
    MPI_Isend(x,1,MPI_DOUBLE,0,11,MPI_COMM_WORLD,&req) ;
    MPI_Send(y,1,MPI_DOUBLE,0,12,MPI_COMM_WORLD) ;
    MPI_Wait(&req,&status) ;
  }
}

int main(int argc, char** argv) {
  int rank ;
  double x,y;
  MPI_Init(0,0);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    x=1.0;
    y=2.0;
  } else {
    x=1.0+rank ;
    y=rank/2.0 ;
  }
  head(&x,&y);
  if (rank == 0) {
    printf(__FILE__ ": rank %i: cumulx=%f \n", rank, x);
    printf(__FILE__ ": rank %i: cumuly=%f \n", rank, y);
  } 
  MPI_Finalize();
  return 0;
}
