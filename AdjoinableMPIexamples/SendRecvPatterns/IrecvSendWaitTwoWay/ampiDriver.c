#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double sBuf,rBuf;
  AMPI_Request r; 
  if (world_rank == 0) {
    sBuf = 3.14;
    AMPI_Irecv(&rBuf, 1, TYPE_MACRO, 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r); /* adjoint needs to wait / nullify */ 
    AMPI_Send (&sBuf, 1, TYPE_MACRO, 1, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD); /* adjoint needs to recv / increment */ 
    printf(__FILE__ ": process 0 received number %f from process 1\n", rBuf);
    AMPI_Wait_ST(&r, &rBuf, MPI_STATUS_IGNORE); /* adjoint needs to isend */
    printf(__FILE__ ": process 0 is done waiting for Irecv\n");
  } else if (world_rank == 1) {
    sBuf = 6.28;
    AMPI_Irecv(&rBuf, 1, TYPE_MACRO, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r); /* adjoint needs to wait / nullify */ 
    AMPI_Send (&sBuf, 1, TYPE_MACRO, 0, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD); /* adjoint needs to recv / increment */ 
    printf(__FILE__ ": process 1 received number %f from process 0\n", rBuf);
    AMPI_Wait_ST(&r, &rBuf, MPI_STATUS_IGNORE); /* adjoint needs to isend */
    printf(__FILE__ ": process 1 is done waiting for Irecv\n");
  } 
  AMPI_Finalize_NT();
  return 0;
}
