#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank,left_rank,right_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  left_rank=(world_rank-1+world_size)%world_size;
  right_rank=(world_rank+1)%world_size;
  double sBuf,rBuf[2];
  MPI_Request r[4]; 
  sBuf = 3.0*world_rank;
  MPI_Isend(&sBuf   , 1, MPI_DOUBLE, left_rank,  0, MPI_COMM_WORLD, &r[0]);
  MPI_Isend(&sBuf   , 1, MPI_DOUBLE, right_rank, 0, MPI_COMM_WORLD, &r[1]);
  MPI_Irecv(&rBuf[0], 1, MPI_DOUBLE, left_rank,  0, MPI_COMM_WORLD, &r[2]);
  MPI_Irecv(&rBuf[1], 1, MPI_DOUBLE, right_rank, 0, MPI_COMM_WORLD, &r[3]);
  MPI_Waitall(4,r,MPI_STATUS_IGNORE);
  printf(__FILE__ ": process %d received number %f from process %d\n", world_rank,rBuf[0],left_rank);
  printf(__FILE__ ": process %d received number %f from process %d\n", world_rank,rBuf[1],right_rank);
  MPI_Finalize();
  return 0;
}
