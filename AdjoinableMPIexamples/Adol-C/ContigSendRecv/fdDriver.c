#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

MPI_Datatype contigMPItype;

int head(double* x, double *y) { 
  MPI_Status s;
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[4] = x[4]*2;
  if (world_rank==0) MPI_Send(x,2,contigMPItype,1,10,MPI_COMM_WORLD);
  if (world_rank==1) MPI_Recv(x,2,contigMPItype,0,10,MPI_COMM_WORLD,&s);
  x[4] = sin(x[4]);
  if (world_rank==1) MPI_Send(x,2,contigMPItype,0,20,MPI_COMM_WORLD);
  if (world_rank==0) MPI_Recv(y,2,contigMPItype,1,20,MPI_COMM_WORLD,&s);
  y[4] = y[4]*3;
  return 0;
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Datatype aTempType;

  double x[6];
  double y[6];
  double yp[6];
  double xv;

  MPI_Type_contiguous(2,
		      MPI_DOUBLE,
		      &aTempType);
  MPI_Type_create_resized(aTempType,0,3*sizeof(double),&contigMPItype); /* send 0, 1, 3, 4, 6, 7,... */
  MPI_Type_commit(&contigMPItype);

  double h;
  xv=3.5;
  h=1.0e-8;
  if (world_rank == 0) {
    x[4] = xv;
    head(x,y);
    printf(__FILE__ ": process 0 got number %f \n", y[4]);
  } else if (world_rank == 1) {
    head(x,y);
  } 
  if (world_rank == 0) {
    x[4] = xv+h;
    head(x,yp);
    printf(__FILE__ ": process 0 got fd value %f \n", (yp[4]-y[4])/h);
  } else if (world_rank == 1) {
    head(x,yp);
  } 
  MPI_Type_free(&aTempType);
  MPI_Type_free(&contigMPItype);
  MPI_Finalize();
  return 0;
}
