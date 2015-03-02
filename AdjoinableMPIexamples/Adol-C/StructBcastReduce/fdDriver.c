#include <stdio.h>
#include <mpi.h>
#include <math.h>

typedef struct {
  double b;
  int d;
  double e;
  char g;
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
  MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[1].b = x[1].b*2;     x[1].e = x[1].e*2;
  MPI_Bcast(x,2,astructMPItype,0,MPI_COMM_WORLD);
  x[1].b = sin(x[1].b);  x[1].e = sin(x[1].e);
  MPI_Reduce(x,y,2,astructMPItype,astructMPIop,0,MPI_COMM_WORLD);
  y[1].b = y[1].b*3;     y[1].e = y[1].e*3;
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Op_create(myop,1,&astructMPIop);
  MPI_Datatype aTempType;

  aStruct x[2];
  aStruct y[2];
  aStruct yp[2];
  aStruct xv;

  int blockcount = 2;
  int blocks[] = {1,1};
  MPI_Datatype types[] = {MPI_DOUBLE,MPI_DOUBLE};
  MPI_Aint displacements[2];
  displacements[0] = (char*)&(x->b) - (char*)(x);
  displacements[1] = (char*)&(x->e) - (char*)(x);
  MPI_Type_create_struct(blockcount,blocks,displacements,types,&aTempType);
  MPI_Type_commit(&aTempType);
  MPI_Type_create_resized(aTempType,0,sizeof(aStruct),&astructMPItype);
  MPI_Type_commit(&astructMPItype);

  double h;
  xv.b=3.5;
  xv.e=3.5;
  h=1.0e-8;
  if (world_rank == 0) {
    x[1].b=xv.b;
    x[1].e=xv.e;
    head(x,y);
    printf(__FILE__ ": process 0 got numbers %f %f\n", y[1].b, y[1].e);
  } else  {
    head(x,y);
  } 
  if (world_rank == 0) {
    x[1].d=xv.d+h;
    x[1].e=xv.e+h;
    head(x,yp);
    printf(__FILE__ ": process 0 got fd value %f\n", (yp[1].e-y[1].e)/h);
  } else  {
    head(x,y);
  } 
  MPI_Type_free(&aTempType);
  MPI_Type_free(&astructMPItype);
  MPI_Op_free(&astructMPIop);
  MPI_Finalize();
  return 0;
}
