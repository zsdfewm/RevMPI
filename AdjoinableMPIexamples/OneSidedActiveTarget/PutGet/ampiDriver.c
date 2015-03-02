#include <stdio.h>
#include "ampi/ampi.h"
#include <math.h>

int head(double* x, double *y) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    AMPI_Win emptyWin;  /* can have added info like a request */
    AMPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &emptyWin); /* in the adjoint is a Win_free */
    AMPI_Win_fence(0, emptyWin);  /* see for the adjoint rules under  MARK 1 in Put/ampiDriver.c  */
    AMPI_Put(x,1,MPI_DOUBLE,1,0,1,MPI_DOUBLE,emptyWin); /* this needs to get the adjoint inte termpoary */
    AMPI_Win_fence(0, emptyWin); /* in the adjoint nothing special */
    /* the other side does something */
    AMPI_Win_fence(0, emptyWin); /* nullify the y_bar */
    AMPI_Get(y,1,MPI_DOUBLE,1,0,1,MPI_DOUBLE,emptyWin); /* Accumulate into the target adjoint */ 
    AMPI_Win_fence(0, emptyWin); /* in the adjoint nothing special */
    AMPI_Win_free(&emptyWin); /* win_create with empty window */
  } else if (world_rank == 1) {
    AMPI_Win localWin;  /* can have added info like a request */
    double local=0.0;
    AMPI_Win_create(&local, sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &localWin); /* in the adjoint is a Win_free */
    AMPI_Win_fence(0, localWin); /* turn this into two fences so the PUT initiated by the adoint of the fence on the other end can go here */
    /* local's value is being put here by rank 0 */
    AMPI_Win_fence(0, localWin);  /* in the adjoint nothing special */
    local=local*2; /* modify the value */
    AMPI_Win_fence(0, localWin); /* turn this into two fences so the PUT initiated by the adoint of the fence on the other end can go here */
    /* local's value is being retrieved from here by rank 0 */ 
    AMPI_Win_fence(0, localWin); /* nothing is done for the adjoint */
    AMPI_Win_free(&localWin); /* win create on local bar */
  } 
}

int main(int argc, char** argv) {
  AMPI_Init(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f \n", y);
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  AMPI_Finalize();
  return 0;
}
