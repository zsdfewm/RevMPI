#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ampi/ampi.h>

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank; 
  const int vSize=3;
  const int root=0;
  double x=3.14,y=0.0;
  int gSize,i,r;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Comm_size(MPI_COMM_WORLD, &gSize);
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
  AMPI_Scatter(buf1, vSize,TYPE_MACRO, buf2, vSize, TYPE_MACRO, root, MPI_COMM_WORLD);
  for (i=0;i<vSize;++i) { 
    buf2[i]*=3.0;
  }
  AMPI_Allgather(buf2, vSize, TYPE_MACRO, buf3, vSize, TYPE_MACRO, MPI_COMM_WORLD);
  for(r=0;r<gSize;++r) { 
    for (i=0;i<vSize;++i) { 
      buf3[r*vSize+i]*=(world_rank+1);
    }
  }
  AMPI_Reduce(buf3,
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
    printf(__FILE__ ": process 0 result is %f \n",y);
    free ((void*)buf1);
  } 
  free ((void*)buf2);
  free ((void*)buf3);
  free ((void*)buf4);
  AMPI_Finalize_NT();
  return 0;
}
