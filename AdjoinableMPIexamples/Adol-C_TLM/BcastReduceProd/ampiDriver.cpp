#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"
#include "ampiHead.h"

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
  AMPI_Finalize_NT();
  return 0;
}
