#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double sBuf,rBuf;
  MPI_Request r; 
  if (world_rank == 0) {
    sBuf = 3.14;
    MPI_Isend(&sBuf, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD,&r);
    MPI_Recv(&rBuf, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 0 received number %f from process 1\n", rBuf);
    MPI_Wait(&r,MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 0 is done waiting for Isend\n");
  } else if (world_rank == 1) {
    sBuf = 6.28;
    MPI_Isend(&sBuf, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,&r);
    MPI_Recv(&rBuf, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 1 received number %f from process 0\n", rBuf);
    MPI_Wait(&r,MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 1 is done waiting for Isend\n");
  } 
  MPI_Finalize();
  return 0;
}
