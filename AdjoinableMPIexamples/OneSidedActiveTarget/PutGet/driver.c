#include <stdio.h>
#include <mpi.h>
#include <math.h>

int head(double* x, double *y) { 
  MPI_Request r; 
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    MPI_Win emptyWin;
    MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &emptyWin);
    MPI_Win_fence(0, emptyWin);
    MPI_Put(x,1,MPI_DOUBLE,1,0,1,MPI_DOUBLE,emptyWin);
    MPI_Win_fence(0, emptyWin);
    /* the other side does something */
    MPI_Win_fence(0, emptyWin);
    MPI_Get(y,1,MPI_DOUBLE,1,0,1,MPI_DOUBLE,emptyWin);
    MPI_Win_fence(0, emptyWin);
    MPI_Win_free(&emptyWin);
  } else if (world_rank == 1) {
    MPI_Win localWin;
    double local=0.0;
    MPI_Win_create(&local, sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &localWin);
    MPI_Win_fence(0, localWin);
    /* local's value is being put here by rank 0 */
    MPI_Win_fence(0, localWin);
    local=local*2; /* modify the value */
    MPI_Win_fence(0, localWin);
    /* local's value is being retrieved from here by rank 0 */
    MPI_Win_fence(0, localWin);
    MPI_Win_free(&localWin);
  } 
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
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
