/*  DESIRED TAPENADE TANGENT USING THE AMPI LIBRARY
    One proc just sends, one proc just receives */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"

void head_d(double* v1, double* v1d, double *v2, double *v2d) {
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double value ;
  double valued ;
  if (world_rank == 0) {
    valued = 0.5*(*v1d)/sqrt(*v1) ;
    value = sqrt(*v1) ;
    TLS_AMPI_Send(&value, &valued, 1, AMPI_ADOUBLE, AMPI_ADOUBLE, 1, 0,
                  AMPI_TO_RECV, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    TLS_AMPI_Recv(&value, &valued, 1, AMPI_ADOUBLE, AMPI_ADOUBLE, 0, 0,
                  AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *v2d = cos(value)*valued ;
    *v2 = sin(value) ;
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  double xd,yd;
  if (world_rank == 0) {
    x=3.5;
    xd=1.0;
    printf(__FILE__ ": process %i sets val  [3.500000==] %f \n", world_rank, x);
    head_d(&x,&xd,&y,&yd);
  } else if (world_rank == 1) {
    head_d(&x,&xd,&y,&yd);
    printf(__FILE__ ": process %i gets val  [0.955327==] %f \n", world_rank, y);
    printf(__FILE__ ": process %i gets diff  [-0.078989==] %f \n", world_rank, yd);
  } 
  AMPI_Finalize_NT();
  return 0;
}
