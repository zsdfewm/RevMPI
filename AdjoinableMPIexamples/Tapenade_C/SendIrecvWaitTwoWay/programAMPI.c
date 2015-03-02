/* Ported to AMPI. No differentiation */
/* Simple Irecv+Send+Wait || Irecv+Wait+Send */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

int head(double* x, double *y) { 
  AMPI_Request r; 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    AMPI_Irecv(y, 1, MPI_DOUBLE, 1, 0,
               AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
    *x=*x*2;
    AMPI_Send(x, 1, MPI_DOUBLE, 1, 0,
              AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
    *y=*y*3;
  } else if (world_rank == 1) {
    double local;
    AMPI_Irecv(&local, 1, MPI_DOUBLE, 0, 0,
               AMPI_FROM_SEND, MPI_COMM_WORLD,&r);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
    local=sin(local);
    AMPI_Send(&local, 1, MPI_DOUBLE, 0, 0,
              AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
  } 
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=2.5;
    printf(__FILE__ ": process %i sends val [ 2.500000==]%f \n", world_rank, x);
    head(&x,&y);
    printf(__FILE__ ": process %i recvs val [-2.876773==]%f \n", world_rank, y);
    y=y+x ;
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  AMPI_Finalize_NT();
  return 0;
}
