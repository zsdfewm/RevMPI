#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

void head(adouble* x, adouble *y) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble number;
  if (world_rank == 0) {
    number = *x*3.14;
    AMPI_Send(&number, 1, AMPI_ADOUBLE, 1, 0, AMPI_TO_RECV, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    AMPI_Recv(&number, 1, AMPI_ADOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *y=number;
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
    xp=1.0;
    x<<=xp;
    head(&x,&y);
  } else if (world_rank == 1) {
    head(&x,&y);
    y>>=yp;
    printf(__FILE__ ": process 1 received number %f from process 0\n",yp);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,0,1,&xp,0);
    fos_reverse(world_rank,0,1,0,&g);
    /* send the gradient value to 1 so we get ordered output */ 
    AMPI_Send(&g,1,MPI_DOUBLE,1,0,AMPI_TO_RECV,MPI_COMM_WORLD);
  } 
  else if (world_rank == 1) {
    tape_doc(world_rank,1,0,0,&yp);
    w=1.0;
    fos_reverse(world_rank,1,0,&w,0);
    /* recv the gradient value from 0 and print it in this rank so we get ordered output 
     with respect to the file redirect done in makefile on the mpirun invocation */ 
    AMPI_Recv(&g, 1, MPI_DOUBLE, 0, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
  }   
  AMPI_Finalize_NT();
  return 0;
}
