#include <stdio.h>
#include <mpi.h>
#include <math.h>

void head(double* x, double *y) { 
  int world_rank;
  int world_size;
  int i,root=0;
  double t;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  if (world_rank==0) {
    *y=1.0;
  }
  MPI_Bcast(x,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  for (i=1;i<world_size;++i)  { 
    if (i==world_rank){ 
      *x*=i;
      MPI_Send(x,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
    }
    if(world_rank==root) { 
      MPI_Recv(&t,1,MPI_DOUBLE,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      *y+=i*t;
    }
    MPI_Barrier(MPI_COMM_WORLD); /* need this to make it deterministic */
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
    printf(__FILE__ ": process 0 got number %e \n", y);
  } else {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
