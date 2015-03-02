#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void head(double* in, double *out) { 
  int world_rank; 
  const int vSize=3;
  const int root=0;
  double x=*in,y=0.0;
  int gSize,i,r;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &gSize);
  double *buf1=NULL;
  double *buf2=(double*)malloc(sizeof(double)*vSize); 
  double *buf3=(double*)malloc(sizeof(double)*vSize*gSize); 
  double *buf4=(double*)malloc(sizeof(double)*vSize*gSize);
  if (world_rank == root) {
    buf1=(double*)malloc(sizeof(double)*vSize*gSize);
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	buf1[r*vSize+i] = x*(i+1)*(r+1);
      }
    }
  }
  MPI_Scatter(buf1, vSize,MPI_DOUBLE, buf2, vSize, MPI_DOUBLE, root, MPI_COMM_WORLD);
  for (i=0;i<vSize;++i) { 
    buf2[i]*=3.0;
  }
  MPI_Allgather(buf2, vSize, MPI_DOUBLE, buf3, vSize, MPI_DOUBLE, MPI_COMM_WORLD);
  for(r=0;r<gSize;++r) { 
    for (i=0;i<vSize;++i) { 
      buf3[r*vSize+i]*=(world_rank+1);
    }
  }
  MPI_Reduce(buf3,
	     buf4,
	     vSize*gSize,
	     MPI_DOUBLE,
	     MPI_SUM,
	     root,
	     MPI_COMM_WORLD);
  if (world_rank == root) {
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	y+=buf4[r*vSize+i];
      }
    }
    free ((void*)buf1);
    *out=y;
  } 
  free ((void*)buf2);
  free ((void*)buf3);
  free ((void*)buf4);
}

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  double x,y;
  if (world_rank == 0) {
    x=3.14;
    head(&x,&y);
    printf(__FILE__ ": process 0 got number %f \n", y);
  } else {
    head(&x,&y);
  } 
  MPI_Finalize();
  return 0;
}
