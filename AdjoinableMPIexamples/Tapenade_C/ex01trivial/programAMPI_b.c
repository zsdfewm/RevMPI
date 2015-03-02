/*  DESIRED TAPENADE ADJOINT USING THE AMPI LIBRARY
    One proc just sends, one proc just receives */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"
#include "ADFirstAidKit/adBuffer.h"

/*
  Differentiation of head in reverse (adjoint) mode:
   gradient     of useful results: *v1 *v2
   with respect to varying inputs: *v1 *v2
   RW status of diff variables: *v1:incr *v2:in-out
   Plus diff mem management of: v1:in v2:in
*/
void head_b(double *v1, double *v1b, double *v2, double *v2b) {
    int world_rank;
    AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    double value;
    double valueb;
    if (world_rank == 0) {
        value = sqrt(*v1);
        FW_AMPI_Send(&value, 1, AMPI_ADOUBLE, 1, 0, AMPI_TO_RECV, MPI_COMM_WORLD);
        BW_AMPI_Send(&valueb, 1, AMPI_ADOUBLE, 1, 0, AMPI_TO_RECV, MPI_COMM_WORLD);
        if (!(*v1==0.0))
            *v1b = *v1b + valueb/(2.0*sqrt(*v1));
    } else if (world_rank == 1) {
        FW_AMPI_Recv(&value, 1, AMPI_ADOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, 
                 MPI_STATUS_IGNORE);
        valueb = cos(value)*(*v2b);
        BW_AMPI_Recv(&valueb, 1, AMPI_ADOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, 
                 MPI_STATUS_IGNORE);
        *v2b = 0.0;
    }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  double xb,yb;
  if (world_rank == 0) {
    xb=0.0;
    x=3.5;
    printf(__FILE__ ": process %i sets val  [3.5000000==]%f\n", world_rank, x);
    head_b(&x,&xb,&y,&yb);
    printf(__FILE__ ": process %i gets diff [-0.078989==]%f\n", world_rank, xb);
  } else if (world_rank == 1) {
    yb=1.0 ;
    y=0.0 ;
    head_b(&x,&xb,&y,&yb);
  } 
  AMPI_Finalize_NT();
  return 0;
}
