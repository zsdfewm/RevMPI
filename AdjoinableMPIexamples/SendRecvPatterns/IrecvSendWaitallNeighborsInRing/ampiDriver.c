#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_size;
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank,left_rank,right_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  left_rank=(world_rank-1+world_size)%world_size;
  right_rank=(world_rank+1)%world_size;
  double sBuf,rBuf[2];
  AMPI_Request r[2]; 
  sBuf = 3.0*world_rank;
  AMPI_Awaitall(2,r,MPI_STATUS_IGNORE); /* adjoint needs to waitall / increment */
  AMPI_Irecv(&rBuf[0], 1, TYPE_MACRO, left_rank,  0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r[0]); /* adjoint needs to send / nullify */
  AMPI_Irecv(&rBuf[1], 1, TYPE_MACRO, right_rank, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r[1]); /* adjoint needs to send / nullify */
  AMPI_Send(&sBuf, 1, TYPE_MACRO, left_rank,  0, AMPI_TO_IRECV_WAITALL, MPI_COMM_WORLD); /* adjoint needs to irecv */
  AMPI_Send(&sBuf, 1, TYPE_MACRO, right_rank, 0, AMPI_TO_IRECV_WAITALL, MPI_COMM_WORLD); /* adjoint needs to irecv */
  AMPI_Waitall(2,r,MPI_STATUS_IGNORE); /* adjoint does nothing */
  printf(__FILE__ ": process %d received number %f from process %d\n", world_rank,rBuf[0],left_rank);
  printf(__FILE__ ": process %d received number %f from process %d\n", world_rank,rBuf[1],right_rank);
  AMPI_Finalize_NT();
  return 0;
}
