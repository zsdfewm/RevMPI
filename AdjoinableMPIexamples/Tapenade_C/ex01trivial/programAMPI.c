/* Program ported to AMPI. Porting should be made by the user
 * in general (e.g. for pairedWith), but could be automated sometimes.
 * Should behave identical to program.c
 * One proc just sends, one proc just receives */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head(double* v1, double *v2) {
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double value ;
  if (world_rank == 0) {
    value = sqrt(*v1) ;
    AMPI_Send(&value, 1, MPI_DOUBLE, 1, 0, AMPI_TO_RECV, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    AMPI_Recv(&value, 1, MPI_DOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *v2 = sin(value) ;
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    printf(__FILE__ ": process %i sets val  [3.500000==] %f \n", world_rank, x);
    head(&x,&y);
  } else if (world_rank == 1) {
    head(&x,&y);
    printf(__FILE__ ": process %i gets val  [0.955327==] %f \n", world_rank, y);
  } 
  AMPI_Finalize_NT();
  return 0;
}
