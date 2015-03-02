#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double sBuf,rBuf;
  int bufferSize;
  MPI_Pack_size(1,MPI_DOUBLE,MPI_COMM_WORLD,&bufferSize);
  bufferSize=bufferSize+MPI_BSEND_OVERHEAD; 
  void* myBuffer;
  myBuffer=malloc(bufferSize);
  MPI_Buffer_attach(myBuffer,bufferSize);
  sBuf = 3.14*(world_rank+1);
  if (world_rank==0) { 
    /* buffered send breaks the deadlock */
    MPI_Bsend(&sBuf, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
    sBuf=0; /* value in buffer remaines unchanged */ 
    MPI_Recv (&rBuf, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 0 received number %f from process 1\n", rBuf);
  } else if (world_rank == 1) {
    /* buffered send breaks the deadlock */
    MPI_Bsend(&sBuf, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    sBuf=0; /* value in buffer remaines unchanged */
    MPI_Recv (&rBuf, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 1 received number %f from process 0\n", rBuf);
  } 
  MPI_Buffer_detach(myBuffer,&bufferSize);
  free(myBuffer);
  MPI_Finalize();
  return 0;
}
