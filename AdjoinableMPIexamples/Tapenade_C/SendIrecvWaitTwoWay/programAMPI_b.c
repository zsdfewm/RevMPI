/*  DESIRED TAPENADE ADJOINT USING THE AMPI LIBRARY
    
    Simple Irecv+Send+Wait || Irecv+Wait+Send */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"
#include "ADFirstAidKit/adBuffer.h"

/*
  Differentiation of head in reverse (adjoint) mode:
   gradient     of useful results: *x *y
   with respect to varying inputs: *x *y
   RW status of diff variables: *x:in-out *y:in-out
   Plus diff mem management of: x:in y:in
*/
void head_b(double *x, double *xb, double *y, double *yb) {
    AMPI_Request r;
    int world_rank;
    AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if (world_rank == 0) {
        FW_AMPI_Irecv(y, 1, AMPI_ADOUBLE, 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
        *x=*x*2;
        FW_AMPI_Send(x, 1, AMPI_ADOUBLE, 1, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
        FW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        ADTOOL_AMPI_Turn(x, xb) ;
        ADTOOL_AMPI_Turn(y, yb) ;
        *yb = 3*(*yb);
        BW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        BW_AMPI_Send(xb, 1, AMPI_ADOUBLE, 1, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
        *xb = 2*(*xb);
        BW_AMPI_Irecv(yb, 1, AMPI_ADOUBLE, 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
    } else if (world_rank == 1) {
        double local;
        double localb;
        FW_AMPI_Irecv(&local, 1, AMPI_ADOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
        FW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        pushreal8(local);
        local = sin(local);
        FW_AMPI_Send(&local, 1, AMPI_ADOUBLE, 0, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
        ADTOOL_AMPI_Turn(&local, &localb) ;
        BW_AMPI_Send(&localb, 1, AMPI_ADOUBLE, 0, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
        popreal8(&local);
        localb = cos(local)*localb;
        BW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        BW_AMPI_Irecv(&localb, 1, AMPI_ADOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
    }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  double xb,yb;
  if (world_rank == 0) {
    yb=1.0;
    xb=0.0;
    x=2.5;
    printf(__FILE__ ": process %i sends val  [2.500000==]%f \n", world_rank, x);
    head_b(&x,&xb,&y,&yb);
    printf(__FILE__ ": process %i recvs diff [1.701973==]%f \n", world_rank, xb);
  } else if (world_rank == 1) {
    head_b(&x,&xb,&y,&yb);
  } 
  AMPI_Finalize_NT();
  return 0;
}
