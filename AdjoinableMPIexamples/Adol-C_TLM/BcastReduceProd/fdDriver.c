#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include "head.h"

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  double xv,h,yp;
  xv=3.5; 
  h=1.0e-8;
  if (world_rank == 0) {
    x=xv;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f\n", y);
  } else  {
    head(&x,&y);
  } 
  if (world_rank == 0) {
    x=xv+h;
    head(&x,&yp);
    printf(__FILE__ ": process 0 got fd value %f\n", (yp-y)/h);
  } else  {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
