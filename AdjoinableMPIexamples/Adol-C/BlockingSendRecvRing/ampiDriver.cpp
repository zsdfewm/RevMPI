#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

void head(adouble* x, adouble *y) { 
  int world_size;
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble token;
  if (world_rank != 0) {
    AMPI_Recv(&token, 1, AMPI_ADOUBLE, world_rank - 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // printf(__FILE__ ": process %d received token %f from process %d\n", world_rank, token.getValue(), world_rank - 1);
  } else {
    token = *x*3.14;
  }
  AMPI_Send(&token, 1, AMPI_ADOUBLE, (world_rank + 1) % world_size, 0, AMPI_TO_RECV, MPI_COMM_WORLD);
  // Now process 0 can receive from the last process.
  if (world_rank == 0) {
    AMPI_Recv(&token, 1, AMPI_ADOUBLE, world_size - 1, 0, AMPI_FROM_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *y=token;
  }  
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_size;
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y;
  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=1.0;
    x<<=xp;
    head(&x,&y);
    y>>=yp;
    printf(__FILE__ ": process %d received token %f from process %d\n", world_rank, yp, world_size - 1);
  } else {
    head(&x,&y);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,1,1,&xp,&yp);
    w=1.0;
    fos_reverse(world_rank,1,1,&w,&g);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
  } else { 
    tape_doc(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return 0;
}
