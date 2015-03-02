#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

void myop(void* in, void* out, int* len, MPI_Datatype* datatype) {
  int i;
  for (i=0;i<*len;i++) {
    ((adouble*)out)[i] += ((adouble*)in)[i];
  }
}

MPI_Op uop;

void head(adouble* x, adouble *y) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank==0)  *x = *x*2;
  AMPI_Bcast(x,1,TYPE_MACRO,0,MPI_COMM_WORLD);
  *x = sin(*x)*(world_rank+1);
  AMPI_Reduce(x,y,1,TYPE_MACRO,uop,0,MPI_COMM_WORLD);
  if (world_rank==0) *y = *y*3;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Op_create_NT(myop,1,&uop);
  adouble x,y;
  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.5;
    x<<=xp;
    head(&x,&y);
    y>>=yp;
    printf(__FILE__ ": process 0 got number %f \n", yp);
  } 
  else {
    head(&x,&y);
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
  AMPI_Op_free_NT(&uop);
  AMPI_Finalize_NT();
  return 0;
}
