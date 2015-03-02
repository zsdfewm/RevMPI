#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

typedef struct {
  int a;
  double d;
  double e;
} aStruct;

MPI_Datatype astructMPItype;

int head(aStruct* x, aStruct *y) { 
  MPI_Status s;
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[1].a = x[1].a*2;  x[1].d = x[1].d*2;
  if (world_rank==0) MPI_Send(x,2,astructMPItype,1,10,MPI_COMM_WORLD);
  if (world_rank==1) MPI_Recv(x,2,astructMPItype,0,10,MPI_COMM_WORLD,&s);
  x[1].a = x[1].a*5;  x[1].d = sin(x[1].d);
  if (world_rank==1) MPI_Send(x,2,astructMPItype,0,20,MPI_COMM_WORLD);
  if (world_rank==0) MPI_Recv(y,2,astructMPItype,1,20,MPI_COMM_WORLD,&s);
  y[1].a = y[1].a*3;  y[1].d = y[1].d*3;
  return 0;
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Datatype aTempType;

  aStruct x[2];
  aStruct y[2];
  aStruct yp[2];
  aStruct xv;

  int blockcount = 2;
  int blocks[] = {1,1}; /* we ignore the 'e' component on purpose */
  MPI_Datatype types[] = {MPI_INT,MPI_DOUBLE};
  MPI_Aint displacements[2];
  displacements[0] = (char*)&(x->a) - (char*)(x);
  displacements[1] = (char*)&(x->d) - (char*)(x);
  MPI_Type_create_struct(blockcount,blocks,displacements,types,&aTempType);
  MPI_Type_commit(&aTempType);
  MPI_Type_create_resized(aTempType,0,sizeof(aStruct),&astructMPItype);
  MPI_Type_commit(&astructMPItype);

  double h;
  xv.a=1;
  xv.d=3.5;
  h=1.0e-8;
  if (world_rank == 0) {
    x[1].a = xv.a;
    x[1].d = xv.d;
    head(x,y);
    printf(__FILE__ ": process 0 got numbers %d %f \n", y[1].a, y[1].d);
  } else if (world_rank == 1) {
    head(x,y);
  } 
  if (world_rank == 0) {
    x[1].a = 1;
    x[1].d = xv.d+h;
    head(x,yp);
    printf(__FILE__ ": process 0 got fd value %f \n", (yp[1].d-y[1].d)/h);
  } else if (world_rank == 1) {
    head(x,yp);
  } 
  MPI_Type_free(&aTempType);
  MPI_Type_free(&astructMPItype);
  MPI_Finalize();
  return 0;
}
