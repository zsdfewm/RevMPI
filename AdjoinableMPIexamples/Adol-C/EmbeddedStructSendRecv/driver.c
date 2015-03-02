#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

typedef struct {
  int a;
  double d;
  double e;
} embedStruct;

typedef struct {
  int a;
  embedStruct estr;
  double e;
} aStruct;

MPI_Datatype embedstructMPItype;
MPI_Datatype astructMPItype;

int head(aStruct* x, aStruct *y) { 
  MPI_Status s;
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[1].estr.a = x[1].estr.a*2;  x[1].estr.d = x[1].estr.d*2;
  if (world_rank==0) MPI_Send(x,2,astructMPItype,1,10,MPI_COMM_WORLD);
  if (world_rank==1) MPI_Recv(x,2,astructMPItype,0,10,MPI_COMM_WORLD,&s);
  x[1].estr.a = x[1].estr.a*5;  x[1].estr.d = sin(x[1].estr.d);
  if (world_rank==1) MPI_Send(x,2,astructMPItype,0,20,MPI_COMM_WORLD);
  if (world_rank==0) MPI_Recv(y,2,astructMPItype,1,20,MPI_COMM_WORLD,&s);
  y[1].estr.a = y[1].estr.a*3;  y[1].estr.d = y[1].estr.d*3;
  return 0;
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Datatype eTempType;
  MPI_Datatype aTempType;

  aStruct x[2];
  aStruct y[2];
  aStruct yp[2];
  aStruct xv;

  int eblockcount = 2;
  int eblocks[] = {1,1};
  MPI_Datatype etypes[] = {MPI_INT,MPI_DOUBLE};
  MPI_Aint edisplacements[2];
  edisplacements[0] = (char*)&((x->estr).a) - (char*)&(x->estr);
  edisplacements[1] = (char*)&((x->estr).d) - (char*)&(x->estr);
  MPI_Type_create_struct(eblockcount,eblocks,edisplacements,etypes,&eTempType);
  MPI_Type_create_resized(eTempType,0,sizeof(embedStruct),&embedstructMPItype);
  MPI_Type_commit(&embedstructMPItype);
  int blockcount = 2;
  int blocks[] = {1,1};
  MPI_Datatype types[] = {MPI_INT,embedstructMPItype};
  MPI_Aint displacements[2];
  displacements[0] = (char*)&(x->a) - (char*)(x);
  displacements[1] = (char*)&(x->estr) - (char*)(x);
  MPI_Type_create_struct(blockcount,blocks,displacements,types,&aTempType);
  MPI_Type_create_resized(aTempType,0,sizeof(aStruct),&astructMPItype);
  MPI_Type_commit(&astructMPItype);

  if (world_rank == 0) {
    x[1].a = 1;
    x[1].estr.a = 1;
    x[1].estr.d = 3.5;
    head(x,y);
    printf(__FILE__ ": process 0 got numbers %d %f \n", y[1].estr.a, y[1].estr.d);
  } else if (world_rank == 1) {
    head(x,y);
  }
  MPI_Type_free(&eTempType);
  MPI_Type_free(&aTempType);
  MPI_Type_free(&embedstructMPItype);
  MPI_Type_free(&astructMPItype);
  MPI_Finalize();
  return 0;
}
