#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"
#include "dco.hpp"

using namespace std;

typedef dco::a1s::type adouble;

int head(adouble* x, adouble *y) { 
  AMPI_Request r; 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    *x=*x*2;
    AMPI_Isend(x, 1, TYPE_MACRO, 1, 0, AMPI_TO_RECV,       MPI_COMM_WORLD,&r);
    AMPI_Recv (y, 1, TYPE_MACRO, 1, 0, AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
    *y=*y*3;
  } else if (world_rank == 1) {
    adouble local;
    AMPI_Recv (&local, 1, TYPE_MACRO, 0, 0, AMPI_FROM_ISEND_WAIT, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local=sin(local);
    AMPI_Isend(&local, 1, TYPE_MACRO, 0, 0, AMPI_TO_RECV, MPI_COMM_WORLD,&r);
    AMPI_Wait(&r,MPI_STATUS_IGNORE);
  } 
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  dco::a1s::global_tape = dco::a1s::tape::create(1e4);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y;
  adouble x_save=0;
  double xp,yp,w, g ;
  if (world_rank == 0) {
    xp=3.5;
    x=xp;
    dco::a1s::global_tape->register_variable(x);
    x_save=x;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f \n", y);
  } 
  else if (world_rank == 1) {
    head(&x,&y);
  } 
  if (world_rank == 0) {
    dco::a1s::set(y, 1., -1);
    dco::a1s::global_tape->interpret_adjoint();
    dco::a1s::get(x_save, g, -1);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
    
  } 
  else if (world_rank == 1) {
    dco::a1s::set(y, 0., -1);
    dco::a1s::global_tape->interpret_adjoint();
  }   
  AMPI_Finalize_NT();
  return 0;
}
