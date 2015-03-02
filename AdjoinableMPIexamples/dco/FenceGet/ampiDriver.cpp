#include <stdio.h>
#include <math.h>
#include "ampi/ampi.h"
#include "dco.hpp"

using namespace std;

typedef dco::a1s::type adouble;



int head(adouble* x, adouble *y) { 
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Win win1, win2;
  if (world_rank == 0) {
    *x=*x*2;
    AMPI_Win_create(x, sizeof(adouble), sizeof(adouble), MPI_INFO_NULL, MPI_COMM_WORLD, &win1);
    AMPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win2);
    printf("Main win1 ptr: %p\n",win1.plainWindow);
    printf("Main win2 ptr: %p\n",win2.plainWindow);
    AMPI_Win_fence(0, win1);
    AMPI_Win_fence(0, win1);
    AMPI_Win_fence(0, win2);
    AMPI_Get(y, 1, TYPE_MACRO, 1, 0, 1, TYPE_MACRO, win2);
    AMPI_Win_fence(0, win2);
    *y=*y*3;
    AMPI_Win_free(&win1);
    AMPI_Win_free(&win2);
  } else if (world_rank == 1) {
    adouble local=0;
    AMPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win1);
    AMPI_Win_create(&local, sizeof(adouble), sizeof(adouble), MPI_INFO_NULL, MPI_COMM_WORLD, &win2);
    printf("Main win1 ptr: %p\n",win1.plainWindow);
    printf("Main win2 ptr: %p\n",win2.plainWindow);
    AMPI_Win_fence(0, win1);
    AMPI_Get(&local, 1, TYPE_MACRO, 0, 0, 1, TYPE_MACRO, win1);
    AMPI_Win_fence(0, win1);
    local=sin(local);
    AMPI_Win_fence(0, win2);
    AMPI_Win_fence(0, win2);
    AMPI_Win_free(&win1);
    AMPI_Win_free(&win2);
  } 
}

int main(int argc, char** argv) {
  AMPI_Init_NT(&argc,&argv);
  dco::a1s::global_tape = dco::a1s::tape::create(1e4);
  dco::a1s::tape::iterator pos0 = dco::a1s::global_tape->get_position();
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
    dco::a1s::global_tape->tape_to_dot(dco::a1s::global_tape->get_position(), pos0, "tape0.dot");
    dco::a1s::set(y, 1., -1);
    dco::a1s::global_tape->interpret_adjoint();
    dco::a1s::get(x_save, g, -1);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
  } 
  else if (world_rank == 1) {
    dco::a1s::global_tape->tape_to_dot(dco::a1s::global_tape->get_position(), pos0, "tape1.dot");
    dco::a1s::set(y, 0., -1);
    dco::a1s::global_tape->interpret_adjoint();
  }   
  AMPI_Finalize_NT();
  return 0;
}
