/*  DESIRED TAPENADE ADJOINT USING THE AMPI LIBRARY
   Simple Isend+Wait || Recv followed by Recv || Isend+Wait */

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
        *x=*x*2;
        FW_AMPI_Isend(x, 1, AMPI_ADOUBLE, 1, 0,
                      AMPI_TO_RECV, MPI_COMM_WORLD, &r);
        FW_AMPI_Recv(y, 1, AMPI_ADOUBLE, 1, 0,
                     AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        FW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        ADTOOL_AMPI_Turn(x, xb) ;
        ADTOOL_AMPI_Turn(y, yb) ;
        *yb = 3*(*yb);
        BW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        BW_AMPI_Recv(yb, 1, AMPI_ADOUBLE, 1, 0,
                     AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        BW_AMPI_Isend(xb, 1, AMPI_ADOUBLE, 1, 0,
                      AMPI_TO_RECV, MPI_COMM_WORLD, &r);
        *xb = 2*(*xb);
        *yb = 0.0;
    } else if (world_rank == 1) {
        double local;
        double localb;
        FW_AMPI_Recv(&local, 1, AMPI_ADOUBLE, 0, 0,
                     AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        pushreal8(local);
        local = sin(local);
        FW_AMPI_Isend(&local, 1, AMPI_ADOUBLE, 0, 0,
                      AMPI_TO_RECV, MPI_COMM_WORLD, &r);
        FW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        ADTOOL_AMPI_Turn(&local, &localb) ;
        BW_AMPI_Wait(&r, MPI_STATUS_IGNORE);
        BW_AMPI_Isend(&localb, 1, AMPI_ADOUBLE, 0, 0,
                      AMPI_TO_RECV, MPI_COMM_WORLD, &r);
        popreal8(&local);
        localb = cos(local)*localb;
        BW_AMPI_Recv(&localb, 1, AMPI_ADOUBLE, 0, 0,
                     AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
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
    x=3.5;
    printf(__FILE__ ": process %i sets val  [3.500000==]%f\n", world_rank, x);
    head_b(&x,&xb,&y,&yb);
    printf(__FILE__ ": process %i gets diff [4.523414==]%f\n", world_rank, xb);
  } else if (world_rank == 1) {
    head_b(&x,&xb,&y,&yb);
  } 
  AMPI_Finalize_NT();
  return 0;
}
