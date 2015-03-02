#include <stdio.h>
#include <mpi.h>
#include <math.h>

void head(double* x, double *y) { 
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double number;
  if (world_rank == 0) {
    number = *x*3.14;
    MPI_Send(&number, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
  } else if (world_rank == 1) {
    MPI_Recv(&number, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *y=number;
  } 
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  double xv,h,yp;
  xv=1.0; 
  h=1.0e-8;
  if (world_rank == 0) {
    x=xv;
    head(&x,&y);
  } else if (world_rank == 1) {
    head(&x,&y);
    printf(__FILE__ ": process 1 received number %f from process 0\n",y);
  } 
  if (world_rank == 0) {
    x=xv+h;
    head(&x,&yp);
  } else if (world_rank == 1) {
    head(&x,&yp);
    printf(__FILE__ ": process 1 received fd value %f\n",(yp-y)/h);
  } 
  MPI_Finalize();
  return 0;
}
