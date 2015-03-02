/* Ported to AMPI. No differentiation */
/* Simple Isend+Wait || Recv followed by Recv || Isend+Wait */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

int head(double* x, double *y) { 
  AMPI_Request r; 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    *x=*x*2;
    AMPI_Isend(x, 1, MPI_DOUBLE, 1, 0,
               AMPI_TO_RECV, MPI_COMM_WORLD, &r);
    AMPI_Recv(y, 1, MPI_DOUBLE, 1, 0,
              AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
    *y=*y*3;
  } else if (world_rank == 1) {
    double local;
    AMPI_Recv(&local, 1, MPI_DOUBLE, 0, 0,
              AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local=sin(local);
    AMPI_Isend(&local, 1, MPI_DOUBLE, 0, 0,
               AMPI_TO_RECV, MPI_COMM_WORLD,&r);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    printf(__FILE__ ": process %i sends val [3.500000==]%f\n", world_rank, x);
    head(&x,&y);
    printf(__FILE__ ": process %i recvs val [1.970960==]%f\n", world_rank, y);
    y=y+x ;
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  AMPI_Finalize_NT();
  return 0;
}
