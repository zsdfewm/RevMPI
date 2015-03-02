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
  double *rBuf=(double*)malloc(sizeof(double)*vSize);
  double *sBuf=NULL;
  if (world_rank == root) {
    sBuf=(double*)malloc(sizeof(double)*vSize*gSize);
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	sBuf[r*vSize+i] = x*(i+1)*(r+1);
      }
    }
  }
  AMPI_Scatter(sBuf, vSize,TYPE_MACRO, rBuf, vSize, TYPE_MACRO, root, MPI_COMM_WORLD);
  for (i=0;i<vSize;++i) { 
    rBuf[i] = 3*(rBuf[i]);
    /* printf(__FILE__ ": process %i computes  %f \n",world_rank, rBuf[i]); */
  }
  AMPI_Gather(rBuf, vSize, TYPE_MACRO, sBuf, vSize, TYPE_MACRO, root, MPI_COMM_WORLD);
  if (world_rank == root) {
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	y+=sBuf[r*vSize+i];
      }
    }
    printf(__FILE__ ": process 0 result is %f \n",y);
    free ((void*)sBuf);
  } 
  free ((void*)rBuf);
  AMPI_Finalize_NT();
  return 0;
}
