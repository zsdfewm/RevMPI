#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_size;
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double token;
  if (world_rank != 0) {
    /* adjoint needs to send / nullify */
    AMPI_Recv(&token, 1, TYPE_MACRO, world_rank - 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* adjoint needs to send / nullify */
    printf(__FILE__ ": process %d received token %f from process %d\n", world_rank, token, world_rank - 1);
  } else {
    token = 3.14;
  }
  AMPI_Send(&token, 1, TYPE_MACRO, (world_rank + 1) % world_size, 0, AMPI_TO_RECV, MPI_COMM_WORLD); /* adjoint needs to receive / increment */
  if (world_rank == 0) {   // Now process 0 can receive from the last process.
    AMPI_Recv(&token, 1, TYPE_MACRO, world_size - 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE); /* adjoint needs to send / nullify */
    printf(__FILE__ ": process %d received token %f from process %d\n", world_rank, token, world_size - 1);
  }  
  AMPI_Finalize_NT();
  return 0;
}
