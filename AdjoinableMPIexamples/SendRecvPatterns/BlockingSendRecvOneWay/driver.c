#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double number;
  if (world_rank == 0) {
    number = 3.14;
    MPI_Send(&number, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    MPI_Recv(&number, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 1 received number %f from process 0\n",
            number);
  } 
  MPI_Finalize();
  return 0;
}
