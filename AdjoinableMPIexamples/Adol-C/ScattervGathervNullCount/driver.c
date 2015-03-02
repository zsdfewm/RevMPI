#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int head(double* in, double *out) { 
  int world_rank; 
  int vSize=3;
  const int root=0;
  const int nullCountRank=3;
  double x=*in,y=0.0;
  int gSize,i,r;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &gSize);
  double *rBuf=0;
  if (world_rank==nullCountRank) { 
    vSize=0;
  }
  else { 
    rBuf=(double*)malloc(sizeof(double)*vSize);
  }
  double *sBuf=NULL;
  int *counts=NULL;
  int *displs=NULL;
  if (world_rank == root) {
    sBuf=(double*)malloc(sizeof(double)*vSize*gSize);
    counts=(int*)malloc(sizeof(int)*gSize);
    displs=(int*)malloc(sizeof(int)*gSize);
    for(r=0;r<gSize;++r) { 
      if (r!=nullCountRank) { 
	for (i=0;i<vSize;++i) { 
	  sBuf[r*vSize+i] = x*(i+1)*(r+1);
	}
	counts[r]=vSize;
      } 
      else counts[r]=0;
      displs[r]=r*vSize;
    }
  }
  MPI_Scatterv(sBuf, counts, displs,MPI_DOUBLE, rBuf, vSize, MPI_DOUBLE, root, MPI_COMM_WORLD);
  for (i=0;i<vSize;++i) { 
    rBuf[i] = 3*(rBuf[i]);
  }
  MPI_Gatherv(rBuf, vSize, MPI_DOUBLE, sBuf, counts, displs, MPI_DOUBLE, root, MPI_COMM_WORLD);
  if (world_rank == root) {
    for(r=0;r<gSize;++r) { 
      if (r!=nullCountRank) { 
	for (i=0;i<vSize;++i) { 
	  y+=sBuf[r*vSize+i];
	}
      }
    }
    free ((void*)sBuf);free((void*)counts); free((void*)displs);
    *out=y;
  } 
  if (rBuf) free ((void*)rBuf);
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
