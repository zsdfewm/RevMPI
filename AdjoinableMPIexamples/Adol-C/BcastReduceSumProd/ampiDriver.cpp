#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

int head(adouble* x, adouble *y, adouble *z) { 
  AMPI_Request r; 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  AMPI_Reduce(x,y,1,TYPE_MACRO,MPI_SUM,0,MPI_COMM_WORLD);
  AMPI_Reduce(x,z,1,TYPE_MACRO,MPI_PROD,0,MPI_COMM_WORLD);
  if (world_rank==0) { 
    *y = *y*3;
    *z = *z*3;
  }
  return 0;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y,z;
  double xp,yp[2],w[2], g[2] ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.5;
    x<<=xp;
    head(&x,&y,&z);
    y>>=yp[0];
    z>>=yp[1];
    printf(__FILE__ ": process 0 got  (%f,%f) \n", yp[0],yp[1]);
  } 
  else {
    head(&x,&y,&z);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,2,1,&xp,yp);
    w[0]=1.0; w[1]=0.0;
    fos_reverse(world_rank,2,1,w,&(g[0]));
    w[0]=0.0; w[1]=1.0;
    fos_reverse(world_rank,2,1,w,&(g[1]));
    printf(__FILE__ ": process 0 got gradient (%f,%f) \n", g[0],g[1]);
  } 
  else {
    tape_doc(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return 0;
}
