#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

MPI_Datatype contigMPItype;

int head(adouble* x, adouble *y) {
  MPI_Status s;
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  x[4] = x[4]*2;
  if (world_rank==0) AMPI_Send(x,2,contigMPItype,1,10,AMPI_TO_RECV,MPI_COMM_WORLD);
  if (world_rank==1) AMPI_Recv(x,2,contigMPItype,0,10,AMPI_FROM_SEND,MPI_COMM_WORLD,&s);
  x[4] = sin(x[4]);
  if (world_rank==1) AMPI_Send(x,2,contigMPItype,0,20,AMPI_TO_RECV,MPI_COMM_WORLD);
  if (world_rank==0) AMPI_Recv(y,2,contigMPItype,1,20,AMPI_FROM_SEND,MPI_COMM_WORLD,&s);
  y[4] = y[4]*3;
  return 0;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Datatype aTempType;

  adouble x[6];
  adouble y[6];

  MPI_Type_contiguous(2,
		      AMPI_ADOUBLE,
		      &aTempType);
  MPI_Type_create_resized(aTempType,0,3*sizeof(adouble),&contigMPItype); /* send 0, 1, 3, 4, 6, 7,... */
  MPI_Type_commit(&contigMPItype);

  trace_on(world_rank,1);
  double xp,yp,w,g;
  if (world_rank == 0) {
    xp=3.5;
    x[4]<<=xp;
    head(x,y);
    y[4]>>=yp;
    printf(__FILE__ ": process 0 got number %f \n", yp);
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
  AMPI_Type_free_NT(&contigMPItype);
  AMPI_Finalize_NT();
  return 0;
}
