#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double number;
  if (world_rank == 0) {
    number = 3.14;
    AMPI_Send(&number, 1, TYPE_MACRO, 1, 0, AMPI_TO_RECV, MPI_COMM_WORLD); /* adjoint needs to do a recv / increment */
  } else if (world_rank == 1) {
    /* adjoint needs to do a send / nullify */
    AMPI_Recv(&number, 1, TYPE_MACRO, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  /* adjoint needs to do a send / nullify */
    printf(__FILE__ ": process 1 received number %f from process 0\n", number);
  } 
  AMPI_Finalize_NT();
  return 0;
}
