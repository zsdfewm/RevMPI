#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int head(double* in, double *out) { 
  int world_rank; 
  const int vSize=3;
  const int root=0;
  double x=*in,y=0.0;
  int gSize,i,r;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &gSize);
  double *rBuf=(double*)MPI_IN_PLACE;
  int rBufSize=vSize;
  if (world_rank != root) rBuf=(double*)malloc(sizeof(double)*vSize);
  double *sBuf=NULL;
  if (world_rank == root) {
    sBuf=(double*)malloc(sizeof(double)*vSize*gSize);
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	sBuf[r*vSize+i] = x*(i+1)*(r+1);
      }
    }
  }
  MPI_Scatter(sBuf, vSize, MPI_DOUBLE, rBuf, rBufSize, MPI_DOUBLE, root, MPI_COMM_WORLD);
  if (world_rank != root)
    for (i=0;i<vSize;++i) { 
      rBuf[i] = 3*(rBuf[i]);
    }
  else
    for (i=0;i<vSize;++i) { 
      sBuf[i] = 3*(sBuf[i]);
    }
  MPI_Gather(rBuf, rBufSize, MPI_DOUBLE, sBuf, vSize, MPI_DOUBLE, root, MPI_COMM_WORLD);
  if (world_rank == root) {
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	y+=sBuf[r*vSize+i];
      }
    }
    free ((void*)sBuf);
    *out=y;
  } 
  if (world_rank != root)  free ((void*)rBuf);
  return 0;
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
  } else {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
