#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_size;
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  AMPI_Get_processor_name(processor_name, &name_len);
  printf(__FILE__ ": processor %s, rank %d out of %d processors\n", processor_name, world_rank, world_size);
  AMPI_Finalize_NT();
  return 0;
}
