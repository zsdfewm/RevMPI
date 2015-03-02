/* Program ported to AMPI. Not differentiated   */
/* Several Send's and a loop of wildcard Recv's */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head(double* x, double *y) {
  int rank, nbProc, i;
  double recvx, recvy ;
  MPI_Status status ;
  AMPI_Request req ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nbProc) ;
  if (rank==0) {
    for (i=1 ; i<nbProc ; ++i) {
      AMPI_Recv(&recvy, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 12,
                AMPI_FROM_SEND, MPI_COMM_WORLD, &status) ;
      *y = *y*recvy ;
      AMPI_Recv(&recvx, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 11,
                AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, &status) ;
      *x = *x+recvx ;
    }
  } else {
    AMPI_Isend(x,1,MPI_DOUBLE,0,11,AMPI_TO_RECV,MPI_COMM_WORLD,&req) ;
    AMPI_Send(y,1,MPI_DOUBLE,0,12,AMPI_TO_RECV,MPI_COMM_WORLD) ;
    AMPI_Wait(&req,&status) ;
  }
}

int main(int argc, char** argv) {
  int rank ;
  double x,y;
  AMPI_Init_NT(0,0);
  AMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    x=1.0;
    y=2.0;
  } else {
    x=1.0+rank ;
    y=rank/2.0 ;
  }
  head(&x,&y);
  if (rank == 0) {
    printf(__FILE__ ": rank %i: cumulx=%f \n", rank, x);
    printf(__FILE__ ": rank %i: cumuly=%f \n", rank, y);
  } 
  AMPI_Finalize_NT();
  return 0;
}
