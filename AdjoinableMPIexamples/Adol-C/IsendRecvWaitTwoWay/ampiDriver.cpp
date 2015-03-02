#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

void head(adouble* x, adouble *y) { 
  AMPI_Request r; 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    *x=*x*2;
    AMPI_Isend(x, 1, TYPE_MACRO, 1, 0, AMPI_TO_RECV,       MPI_COMM_WORLD,&r);
    AMPI_Recv (y, 1, TYPE_MACRO, 1, 0, AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
    *y=*y*3;
  } else if (world_rank == 1) {
    adouble local;
    AMPI_Recv (&local, 1, TYPE_MACRO, 0, 0, AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local=sin(local);
    AMPI_Isend(&local, 1, TYPE_MACRO, 0, 0, AMPI_TO_RECV, MPI_COMM_WORLD,&r);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
  } 
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y;
  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.5;
    x<<=xp;
    head(&x,&y);
    y>>=yp;
    printf(__FILE__ ": process 0 got number %f \n", yp);
  } 
  else if (world_rank == 1) {
    head(&x,&y);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,1,1,&xp,&yp);
    w=1.0;
    fos_reverse(world_rank,1,1,&w,&g);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
  } 
  else if (world_rank == 1) {
    tape_doc(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return 0;
}
