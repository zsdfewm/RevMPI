#include <stdio.h>
#include <mpi.h>

void head(double* x, double *y) { 
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double token;
  if (world_rank != 0) {
    MPI_Recv(&token, 1, MPI_DOUBLE, world_rank - 1, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    /* printf(__FILE__ ": process %d received token %f from process %d\n", 
           world_rank, token, world_rank - 1); */
  } else {
    token = *x*3.14;
  }
  MPI_Send(&token, 1, MPI_DOUBLE, (world_rank + 1) % world_size,
           0, MPI_COMM_WORLD);
  /* Now process 0 can receive from the last process. */
  if (world_rank == 0) {
    MPI_Recv(&token, 1, MPI_DOUBLE, world_size - 1, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    *y=token;
  }  
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=1.0;
    head(&x,&y);
    printf(__FILE__ ": process %d received token %f from process %d\n",
           world_rank, y, world_size - 1);
  } else {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
