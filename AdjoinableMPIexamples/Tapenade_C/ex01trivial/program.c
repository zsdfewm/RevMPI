/* One proc just sends, one proc just receives */

#include <mpi.h>

#include <stdio.h>
#include <math.h>

void head(double* v1, double *v2) {
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double value ;
  if (world_rank == 0) {
    value = sqrt(*v1) ;
    MPI_Send(&value, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    MPI_Recv(&value, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *v2 = sin(value) ;
  }
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    printf(__FILE__ ": process %i sets val  [3.500000==] %f \n", world_rank, x);
    head(&x,&y);
  } else if (world_rank == 1) {
    head(&x,&y);
    printf(__FILE__ ": process %i gets val  [0.955327==] %f \n", world_rank, y);
  } 
  MPI_Finalize();
  return 0;
}
