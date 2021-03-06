#include <stdio.h>
#include <mpi.h>
#include <math.h>

int head(double* x, double *y) { 
  int world_rank;
  MPI_Win win1, win2;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank == 0) {
    *x=*x*2;
    MPI_Win_create(x, sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win1);
    MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win2);
    MPI_Win_fence(0, win1);
    MPI_Win_fence(0, win1);
    MPI_Win_fence(0, win2);
    MPI_Get(y, 1, MPI_DOUBLE, 1, 0, 1, MPI_DOUBLE, win2);
    MPI_Win_fence(0, win2);
    *y=*y*3;
    MPI_Win_free(&win1);
    MPI_Win_free(&win2);
  } else if (world_rank == 1) {
    double local;
    MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win1);
    MPI_Win_create(&local, sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win2);
    MPI_Win_fence(0, win1);
    MPI_Get(&local, 1, MPI_DOUBLE, 0, 0, 1, MPI_DOUBLE, win1);
    MPI_Win_fence(0, win1);
    local=sin(local);
    MPI_Win_fence(0, win2);
    MPI_Win_fence(0, win2);
    MPI_Win_free(&win1);
    MPI_Win_free(&win2);
  } 
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.5;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f \n", y);
  } else if (world_rank == 1) {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
