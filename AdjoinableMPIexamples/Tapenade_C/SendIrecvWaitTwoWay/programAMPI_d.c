/*  DESIRED TAPENADE TANGENT USING THE AMPI LIBRARY */
/* Simple Irecv+Send+Wait || Irecv+Wait+Send */

#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"

void head_d(double *x, double *xd, double *y, double *yd) {
  AMPI_Request r;
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    TLS_AMPI_Irecv(y, yd, 1, AMPI_ADOUBLE, AMPI_ADOUBLE, 1, 0,
                   AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
    *xd=*xd*2;
    *x=*x*2;
    TLS_AMPI_Send(x, xd, 1, AMPI_ADOUBLE, AMPI_ADOUBLE, 1, 0,
                  AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
    TLS_AMPI_Wait(&r,MPI_STATUS_IGNORE);
    *yd=*yd*3;
    *y=*y*3;
  } else if (world_rank == 1) {
    double local;
    double locald;
    TLS_AMPI_Irecv(&local, &locald, 1, AMPI_ADOUBLE, AMPI_ADOUBLE, 0, 0,
                   AMPI_FROM_SEND, MPI_COMM_WORLD, &r);
    TLS_AMPI_Wait(&r,MPI_STATUS_IGNORE);
    locald=cos(local)*locald ;
    local=sin(local);
    TLS_AMPI_Send(&local, &locald, 1, AMPI_ADOUBLE, AMPI_ADOUBLE, 0, 0,
                  AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD);
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  double xd,yd;
  if (world_rank == 0) {
    xd=1.0;
    x=2.5;
    printf(__FILE__ ": process %i sends val [2.500000==]%f\n", world_rank, x);
    head_d(&x,&xd,&y,&yd);
    printf(__FILE__ ": process %i recvs val [-2.876773==]%f\n", world_rank, y);
    printf(__FILE__ ": process %i recvs diff [1.701973==]%f\n", world_rank, yd);
    y=y+x ;
  }else if (world_rank == 1) {
    head_d(&x,&xd,&y,&yd);
  }
  AMPI_Finalize_NT();
  return 0;
}
