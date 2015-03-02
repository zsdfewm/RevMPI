#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double sBuf,rBuf;
  AMPI_Request sr,rr; 
  if (world_rank == 0) {
    sBuf = 3.14;
    AMPI_Isend(&sBuf, 1, TYPE_MACRO, 1, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD, &sr); /* adjoint needs to wait / increment */
    AMPI_Irecv(&rBuf, 1, TYPE_MACRO, 1, 0, AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, &rr); /* adjoint needs to wait / nullify */
    AMPI_Wait_ST(&sr, &sBuf, MPI_STATUS_IGNORE); /* adjoint needs to Irecv */
    printf(__FILE__ ": process 0 is done waiting for Isend\n");
    AMPI_Wait_ST(&rr, &rBuf, MPI_STATUS_IGNORE); /* adjoint needs to Isend */
    printf(__FILE__ ": process 0 received number %f from process 1\n", rBuf);
  } else if (world_rank == 1) {
    sBuf = 6.28;
    AMPI_Isend(&sBuf, 1, TYPE_MACRO, 0, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD, &sr); /* adjoint needs to wait / increment */
    AMPI_Irecv(&rBuf, 1, TYPE_MACRO, 0, 0, AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, &rr); /* adjoint needs to wait / nullify */
    AMPI_Wait_ST(&sr, &sBuf, MPI_STATUS_IGNORE); /* adjoint needs to Irecv */
    printf(__FILE__ ": process 1 is done waiting for Isend\n");
    AMPI_Wait_ST(&rr, &rBuf, MPI_STATUS_IGNORE); /* adjoint needs to Isend */
    printf(__FILE__ ": process 1 received number %f from process 0\n", rBuf);
  } 
  AMPI_Finalize_NT();
  return 0;
}
