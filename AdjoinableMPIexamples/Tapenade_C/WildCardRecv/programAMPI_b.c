/* Program ported to AMPI. Tangent differentiation.   */
/* Several Send's and a loop of wildcard Recv's */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head_b(double* x, double* xb, double *y, double *yb) {
  int rank, nbProc, i;
  double recvx, recvy ;
  double recvxb, recvyb ;
  MPI_Status status ;
  AMPI_Request req ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nbProc) ;
  if (rank==0) {
    for (i=1 ; i<nbProc ; ++i) {
      pushreal8(recvy) ;
      FW_AMPI_Recv(&recvy, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 12,
                   AMPI_FROM_SEND, MPI_COMM_WORLD, &status) ;
      pushreal8(*y) ;
      *y = *y*recvy ;
      FW_AMPI_Recv(&recvx, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 11,
                   AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, &status) ;
      *x = *x+recvx ;
    }
    ADTOOL_AMPI_Turn(x,xb) ;
    ADTOOL_AMPI_Turn(y,yb) ;
    ADTOOL_AMPI_Turn(&recvx,&recvxb) ;
    ADTOOL_AMPI_Turn(&recvy,&recvyb) ;
    recvxb = 0.0 ;
    recvyb = 0.0 ;
    for (i=nbProc-1 ; i>=1 ; --i) {
      recvxb = recvxb+*xb ;
      BW_AMPI_Recv(&recvxb, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 11,
                   AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, &status) ;
      popreal8(y) ;
      recvyb = recvyb + *y * *yb ;
      *yb = recvy * *yb ;
      popreal8(&recvy) ;
      BW_AMPI_Recv(&recvyb, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 12,
                   AMPI_FROM_SEND, MPI_COMM_WORLD, &status) ;
    }
  } else {
    FW_AMPI_Isend(x,1,MPI_DOUBLE,0,11,AMPI_TO_RECV,MPI_COMM_WORLD,&req) ;
    FW_AMPI_Send(y,1,MPI_DOUBLE,0,12,AMPI_TO_RECV,MPI_COMM_WORLD) ;
    FW_AMPI_Wait(&req,&status) ;
    ADTOOL_AMPI_Turn(x,xb) ;
    ADTOOL_AMPI_Turn(y,yb) ;
    BW_AMPI_Wait(&req,&status) ;
    BW_AMPI_Send(yb,1,MPI_DOUBLE,0,12,AMPI_TO_RECV,MPI_COMM_WORLD) ;
    BW_AMPI_Isend(xb,1,MPI_DOUBLE,0,11,AMPI_TO_RECV,MPI_COMM_WORLD,&req) ;
  }
}

int main(int argc, char** argv) {
  int rank ;
  double xb,yb;
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
  xb=1.0 ;
  yb=1.0 ;
  head_b(&x,&xb,&y,&yb);
  globalsumdiff = 0.0 ;
  MPI_Reduce(&xb,&tmpsumdiff,1,MPI_DOUBLE,MPI_SUM,
             0,MPI_COMM_WORLD) ;
  globalsumdiff = globalsumdiff+tmpsumdiff ;
  MPI_Reduce(&yb,&tmpsumdiff,1,MPI_DOUBLE,MPI_SUM,
             0,MPI_COMM_WORLD) ;
  globalsumdiff = globalsumdiff+tmpsumdiff ;
  if (rank==0) {
    printf("rank 0: sum of all input gradients [27.000==] %f\n", globalsumdiff) ;
  }
  AMPI_Finalize_NT();
  return 0;
}
