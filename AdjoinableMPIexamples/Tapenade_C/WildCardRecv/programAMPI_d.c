/* Program ported to AMPI. Tangent differentiation.   */
/* Several Send's and a loop of wildcard Recv's */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head_d(double* x, double* xd, double *y, double *yd) {
  int rank, nbProc, i;
  double recvx, recvy ;
  double recvxd, recvyd ;
  MPI_Status status ;
  AMPI_Request req ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nbProc) ;
  if (rank==0) {
    for (i=1 ; i<nbProc ; ++i) {
      TLS_AMPI_Recv(&recvy, &recvyd, 1, MPI_DOUBLE, MPI_DOUBLE, MPI_ANY_SOURCE, 12,
                AMPI_FROM_SEND, MPI_COMM_WORLD, &status) ;
      *yd = *yd*recvy + *y*recvyd ;
      *y = *y*recvy ;
      TLS_AMPI_Recv(&recvx, &recvxd, 1, MPI_DOUBLE, MPI_DOUBLE, MPI_ANY_SOURCE, 11,
                AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, &status) ;
      *xd = *xd+recvxd ;
      *x = *x+recvx ;
    }
  } else {
    TLS_AMPI_Isend(x,xd,1,MPI_DOUBLE,MPI_DOUBLE,0,11,AMPI_TO_RECV,MPI_COMM_WORLD,&req) ;
    TLS_AMPI_Send(y,yd,1,MPI_DOUBLE,MPI_DOUBLE,0,12,AMPI_TO_RECV,MPI_COMM_WORLD) ;
    TLS_AMPI_Wait(&req,&status) ;
  }
}

int main(int argc, char** argv) {
  int rank ;
  double xd,yd;
  double x,y;
  double tmpsumdiff,globalsumdiff ;
  AMPI_Init_NT(0,0);
  AMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    x=1.0;
    y=2.0;
  } else {
    x=1.0+rank ;
    y=rank/2.0 ;
  }
  xd=1.0 ;
  yd=1.0 ;
  head_d(&x,&xd,&y,&yd);
  if (rank == 0) {
    printf(__FILE__ ": rank %i: cumulx=%f \n", rank, x);
    printf(__FILE__ ": rank %i: cumuly=%f \n", rank, y);
  } 
  globalsumdiff = 0.0 ;
  MPI_Reduce(&xd,&tmpsumdiff,1,MPI_DOUBLE,MPI_SUM,
             0,MPI_COMM_WORLD) ;
  globalsumdiff = globalsumdiff+tmpsumdiff ;
  MPI_Reduce(&yd,&tmpsumdiff,1,MPI_DOUBLE,MPI_SUM,
             0,MPI_COMM_WORLD) ;
  globalsumdiff = globalsumdiff+tmpsumdiff ;
  if (rank==0) {
    printf("rank 0: sum of all output tangents [27.000==] %f\n", globalsumdiff) ;
  }
  AMPI_Finalize_NT();
  return 0;
}
