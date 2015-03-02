#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

typedef struct {
  int a;
  adouble d;
  double e;
} aStruct;

MPI_Datatype astructMPItype;

int head(aStruct* x, aStruct *y) { 
  MPI_Status s;
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[1].a = x[1].a*2;  x[1].d = x[1].d*2;
  if (world_rank==0) AMPI_Send(x,2,astructMPItype,1,10,AMPI_TO_RECV,MPI_COMM_WORLD);
  if (world_rank==1) AMPI_Recv(x,2,astructMPItype,0,10,AMPI_FROM_SEND,MPI_COMM_WORLD,&s);
  x[1].a = x[1].a*5;  x[1].d = sin(x[1].d);
  if (world_rank==1) AMPI_Send(x,2,astructMPItype,0,20,AMPI_TO_RECV,MPI_COMM_WORLD);
  if (world_rank==0) AMPI_Recv(y,2,astructMPItype,1,20,AMPI_FROM_SEND,MPI_COMM_WORLD,&s);
  y[1].a = y[1].a*3;  y[1].d = y[1].d*3;
  return 0;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Datatype aTempType;

  aStruct x[2];
  aStruct y[2];
  
  int blockcount = 2;
  int blocks[] = {1,1}; /* we ignore the 'e' component on purpose */
  MPI_Datatype types[] = {MPI_INT,AMPI_ADOUBLE};
  MPI_Aint displacements[2];
  displacements[0] = (char*)&(x->a) - (char*)(x);
  displacements[1] = (char*)&(x->d) - (char*)(x);
  AMPI_Type_create_struct_NT(blockcount,blocks,displacements,types,&aTempType);
  AMPI_Type_commit_NT(&aTempType);
  AMPI_Type_create_resized_NT(aTempType,0,sizeof(aStruct),&astructMPItype);
  AMPI_Type_commit_NT(&astructMPItype);

  trace_on(world_rank,1);
  double xp,yp,w,g;
  if (world_rank == 0) {
    x[0].a = 2;
    x[1].a = 1;
    xp=3.5;
    x[1].d<<=xp;
    head(x,y);
    y[1].d>>=yp;
    printf(__FILE__ ": process 0 got numbers %d %f \n", y[1].a, yp);
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
  AMPI_Type_free_NT(&aTempType);
  AMPI_Type_free_NT(&astructMPItype);
  AMPI_Finalize_NT();
  return 0;
}
