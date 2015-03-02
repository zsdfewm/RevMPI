#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

typedef struct {
  double b;
  adouble e;
} aStruct;

MPI_Datatype astructMPItype;

void myop(void* in, void* out, int* len, MPI_Datatype* datatype) {
  int i;
  for (i=0;i<*len;i++) {
    ((aStruct*)out)[i].b += ((aStruct*)in)[i].b;
    ((aStruct*)out)[i].e *= ((aStruct*)in)[i].e;
  }
}

MPI_Op astructMPIop;

void head(aStruct* x, aStruct *y) { 
  MPI_Status s;
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[1].b = x[1].b*2;     x[1].e = x[1].e*2;
  AMPI_Bcast(x,2,astructMPItype,0,MPI_COMM_WORLD);
  x[1].b = sin(x[1].b);  x[1].e = sin(x[1].e);
  AMPI_Reduce(x,y,2,astructMPItype,astructMPIop,0,MPI_COMM_WORLD);
  y[1].b = y[1].b*3;     y[1].e = y[1].e*3;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Op_create_NT(myop,1,&astructMPIop);
  MPI_Datatype aTempType;

  aStruct x[2];
  aStruct y[2];

  int blockcount = 2;
  int blocks[] = {1,1};
  MPI_Datatype types[] = {MPI_DOUBLE,TYPE_MACRO};
  MPI_Aint displacements[2];
  displacements[0] = (char*)&(x->b) - (char*)(x);
  displacements[1] = (char*)&(x->e) - (char*)(x);
  AMPI_Type_create_struct_NT(blockcount,blocks,displacements,types,&aTempType);
  AMPI_Type_commit_NT(&aTempType);
  AMPI_Type_create_resized_NT(aTempType,0,sizeof(aStruct),&astructMPItype);
  AMPI_Type_commit_NT(&astructMPItype);

  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.5;
    x[0].b = xp;
    x[1].b = xp;
    x[0].e = xp;
    x[1].e<<=xp;
    head(x,y);
    y[1].e>>=yp;
    printf(__FILE__ ": process 0 got numbers %f %f \n", y[1].b, yp);
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
  AMPI_Op_free_NT(&astructMPIop);
  AMPI_Finalize_NT();
  return 0;
}
