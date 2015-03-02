#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

void head(adouble* x, adouble *y) { 
  int world_rank;
  int world_size;
  int i,root=0;
  adouble t;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  if (world_rank==0) {
    *y=1.0;
  }
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  for (i=1;i<world_size;++i)  { 
    if (i==world_rank){ 
      *x*=i;
      AMPI_Send(x,1,TYPE_MACRO,0,0,AMPI_TO_RECV,MPI_COMM_WORLD);
    }
    if(world_rank==root) { 
      AMPI_Recv(&t,1,TYPE_MACRO,MPI_ANY_SOURCE,MPI_ANY_TAG,AMPI_FROM_SEND,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      *y+=i*t;
    }
    AMPI_Barrier(MPI_COMM_WORLD); /* need this to make it deterministic */
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y;
  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.5;
    x<<=xp;
    head(&x,&y);
    y>>=yp;
    printf(__FILE__ ": process 0 got number %e \n", yp);
  } 
  else {
    head(&x,&y);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,1,1,&xp,&yp);
    w=1.0;
    fos_reverse(world_rank,1,1,&w,&g);
    printf(__FILE__ ": process 0 got gradient %e \n", g);
  } 
  else {
    tape_doc(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return 0;
}
