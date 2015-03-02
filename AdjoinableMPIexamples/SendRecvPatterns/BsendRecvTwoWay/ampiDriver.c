#include <stdio.h>
#include <stdlib.h>
#include "ampi/ampi.h"

/**
 * \file symmetric and homogeneous bsend pattern
 * we need symmetry: each process sends and receives; this implies that the buffer remains attached at least until after 
 * the recv to make sure no deadlock can occur (detach may return only after the send operation has finished); in turn 
 * this means that the buffer will be attached for the adjoint counterpart of the recv; Or as is the case in this example 
 * we handle the buffer outside of the adjoinable section; 
 * we need homogeneity: the sent and received messages are of the same size ensuring that the buffer will be large enough
 */
int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double sBuf,rBuf;
  int bufferSize;
  AMPI_Pack_size(1, TYPE_MACRO, MPI_COMM_WORLD, &bufferSize);
  bufferSize=bufferSize+MPI_BSEND_OVERHEAD; 
  void* myBuffer;
  myBuffer=malloc(bufferSize);
  AMPI_Buffer_attach_NT(myBuffer,bufferSize);
  sBuf = 3.14*(world_rank+1);
  /* start adjoinable section +++++++++++++++++++++++++++++++++ */
  if (world_rank==0) { 
    /* buffered send breaks the deadlock */
    AMPI_Bsend(&sBuf, 1, TYPE_MACRO, 1, 0, AMPI_TO_RECV,  MPI_COMM_WORLD); /* adjoint needs to recv / increment */
    sBuf=0; /* value in buffer remaines unchanged */ 
    AMPI_Recv (&rBuf, 1, TYPE_MACRO, 1, 0, AMPI_FROM_BSEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* adjoint needs to bsend / nullify */
    printf(__FILE__ ": process 0 received number %f from process 1\n", rBuf);
  } else if (world_rank == 1) {
    /* buffered send breaks the deadlock */
    AMPI_Bsend(&sBuf, 1, TYPE_MACRO, 0, 0, AMPI_TO_RECV,  MPI_COMM_WORLD); /* adjoint needs to recv / increment */
    sBuf=0; /* value in buffer remaines unchanged */
    AMPI_Recv (&rBuf, 1, TYPE_MACRO, 0, 0, AMPI_FROM_BSEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* adjoint needs to bsend / nullify */
    printf(__FILE__ ": process 1 received number %f from process 0\n", rBuf);
  } 
  /* end adjoinable section +++++++++++++++++++++++++++++++++++ */
  AMPI_Buffer_detach_NT(myBuffer,&bufferSize);
  free(myBuffer); 
  AMPI_Finalize_NT();
  return 0;
}
