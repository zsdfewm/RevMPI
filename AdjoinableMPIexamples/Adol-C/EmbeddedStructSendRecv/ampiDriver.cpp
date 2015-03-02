#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

typedef struct {
  int a;
  adouble d;
  double e;
} embedStruct;

typedef struct {
  int a;
  embedStruct estr;
  double e;
  embedStruct foo[4];
} aStruct;

MPI_Datatype embedstructMPItype;
MPI_Datatype astructMPItype;

int head(aStruct* x, aStruct *y) { 
  MPI_Status s;
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[1].estr.a = x[1].estr.a*2;  x[1].estr.d = x[1].estr.d*2;
  if (world_rank==0) AMPI_Send(x,2,astructMPItype,1,10,AMPI_TO_RECV,MPI_COMM_WORLD);
  if (world_rank==1) AMPI_Recv(x,2,astructMPItype,0,10,AMPI_FROM_SEND,MPI_COMM_WORLD,&s);
  x[1].estr.a = x[1].estr.a*5;  x[1].estr.d = sin(x[1].estr.d);
  if (world_rank==1) AMPI_Send(x,2,astructMPItype,0,20,AMPI_TO_RECV,MPI_COMM_WORLD);
  if (world_rank==0) AMPI_Recv(y,2,astructMPItype,1,20,AMPI_FROM_SEND,MPI_COMM_WORLD,&s);
  y[1].estr.a = y[1].estr.a*3;  y[1].estr.d = y[1].estr.d*3;
  return 0;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Datatype eTempType;
  MPI_Datatype aTempType;

  aStruct x[2];
  aStruct y[2];

  int eblockcount = 2;
  int eblocks[] = {1,1};
  MPI_Datatype etypes[] = {MPI_INT,AMPI_ADOUBLE};
  MPI_Aint edisplacements[2];
  edisplacements[0] = (char*)&((x->estr).a) - (char*)&(x->estr);
  edisplacements[1] = (char*)&((x->estr).d) - (char*)&(x->estr);
  AMPI_Type_create_struct_NT(eblockcount,eblocks,edisplacements,etypes,&eTempType);
  AMPI_Type_create_resized_NT(eTempType,0,sizeof(embedStruct),&embedstructMPItype);
  AMPI_Type_commit_NT(&embedstructMPItype);
  int blockcount = 3;
  int blocks[] = {1,1,4};
  MPI_Datatype types[] = {MPI_INT,embedstructMPItype,embedstructMPItype};
  MPI_Aint displacements[3];
  displacements[0] = (char*)&(x->a) - (char*)(x);
  displacements[1] = (char*)&(x->estr) - (char*)(x);
  displacements[2] = (char*)&(x->foo[0]) - (char*)(x); /* we don't care about the value in foo, but we need to
							  include it in the datatype to ensure that the contiguity
							  check asserts true */
  AMPI_Type_create_struct_NT(blockcount,blocks,displacements,types,&aTempType);
  AMPI_Type_create_resized_NT(aTempType,0,sizeof(aStruct),&astructMPItype);
  AMPI_Type_commit_NT(&astructMPItype);

  trace_on(world_rank,1);
  double xp,yp,w,g;
  if (world_rank == 0) {
    x[0].estr.a = 2;
    x[1].estr.a = 1;
    xp=3.5;
    x[1].estr.d<<=xp;
    head(x,y);
    y[1].estr.d>>=yp;
    printf(__FILE__ ": process 0 got numbers %d %f \n", y[1].estr.a, yp);
  } 
  else {
    head(x,y);
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
  AMPI_Type_free_NT(&eTempType);
  AMPI_Type_free_NT(&aTempType);
  AMPI_Type_free_NT(&embedstructMPItype);
  AMPI_Type_free_NT(&astructMPItype);
  AMPI_Finalize_NT();
  return 0;
}
