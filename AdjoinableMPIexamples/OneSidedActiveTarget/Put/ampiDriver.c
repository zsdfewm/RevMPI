#include <stdio.h>
#include "ampi/ampi.h"
#include <math.h>

int head(double* x, double *y) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    AMPI_Win emptyWin,yWin;  /* can have added info like a request */
    AMPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &emptyWin); /* in the adjoint is a Win_free */ 
    AMPI_Win_fence(0, emptyWin); /* see for the adjoint rules below;  MARK 1  */
    /* in the adjoint, this need to do the increment on the x_bar , 
       make sure the assertion if any is adjusted so it acts as a proper fence in the adjoint
       let this fence complete THEN 
       PUT a 0 to nullify the local_bar on the other end  - note that offset to get to the right spot should be 
       just the same as the offset used in the adjoint of the PUT, i.e. the unchanged offset value. 
       then do another fence to let the PUT complete 
    */
    AMPI_Put(x,1,MPI_DOUBLE,1,0,1,MPI_DOUBLE,emptyWin); /* this needs to get the adjoint into a temporary, also below */
    /* the displacement on the target side should not have to be changed provided that 
       1. the target window unit size is adjusted to the size of the derivative data 
       2. in operator overloading it is ensured that the hash addresses of all elements in the target window are consecutive 
       */
    AMPI_Win_fence(0, emptyWin); /* in the adjoint nothing special */
    AMPI_Win_free(&emptyWin); /* in the adjoint win_create (empty in this case too)  */ 
    AMPI_Win_create(y, sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &yWin); /* in the adjoint a Win_free */
    AMPI_Win_fence(0, yWin); /* in the adjoint this needs to nullify the y_bar */
    /* y's value is being put here by rank 1 */  /* in the adjoint somebody has to nullify */ 
    AMPI_Win_fence(0, yWin); /* in the adjoint nothing special */ 
    AMPI_Win_free(&yWin); /* in the adjoint win_create with y_bar as the base address */
  } else if (world_rank == 1) {
    AMPI_Win emptyWin,localWin;
    double local=0.0;
    AMPI_Win_create(&local, sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &localWin); /* in the adjoint is a Win_free */ 
    AMPI_Win_fence(0, localWin); /* turn this into two fences so the PUT initiated by the adoint of the fence on the other end can go here */
    /* local's value is being put here by rank 0 */ /* nothing is done for the adjoint */
    AMPI_Win_fence(0, localWin); /* in the adjoint nothing special */
    AMPI_Win_free(&localWin); /* in the adjoint a win create on local_bar */
    local=local*2; /* modify the value */ /* regular adjoint */
    AMPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &emptyWin); /* win free */
    AMPI_Win_fence(0, emptyWin);   /* see above under MARK 1 */
    AMPI_Put(&local,1,MPI_DOUBLE,0,0,1,MPI_DOUBLE,emptyWin); /* get the adjoint into a temporary */
    AMPI_Win_fence(0, emptyWin);  /* in the adjoint nothing special */
    AMPI_Win_free(&emptyWin); /* in the adjoint win_create (empty in this case too)  */ 
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
