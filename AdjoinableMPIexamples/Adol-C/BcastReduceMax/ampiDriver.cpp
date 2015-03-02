#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

MPI_Op op = MPI_MAX;
/* ^ change MPI_SUM for results of another op */

int head(adouble* x, adouble *y) { 
  AMPI_Request r; 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0) *x = *x*2;
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  *x = sin(*x*(world_rank+1));
  AMPI_Reduce(x,y,1,AMPI_ADOUBLE,op,0,MPI_COMM_WORLD);
  if (world_rank==0) *y = *y*3;
  return 0;
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
  else {
    head(&x,&y);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,1,1,&xp,&yp);
    w=1.0;
    fos_reverse(world_rank,1,1,&w,&g);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
  } 
  else {
    tape_doc(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return 0;
}
