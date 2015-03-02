#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

int head(adouble* in, adouble *out) { 
  int world_rank; 
  const int vSize=3;
  const int root=0;
  adouble x=*in,y=0.0;
  int gSize,i,r;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Comm_size(MPI_COMM_WORLD, &gSize);
  ensureContiguousLocations(vSize);
  adouble *rBuf=new adouble[vSize];
  adouble *sBuf=NULL;
  int *counts=NULL;
  int *displs=NULL;
  if (world_rank == root) {
    ensureContiguousLocations(vSize*gSize);
    sBuf=new adouble[vSize*gSize];
    counts=new int[gSize];
    displs=new int[gSize];
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	sBuf[r*vSize+i] = x*(i+1)*(r+1);
      }
      counts[r]=vSize;
      displs[r]=r*vSize;
    }
  }
  AMPI_Scatterv(sBuf, counts, displs,TYPE_MACRO, rBuf, vSize, TYPE_MACRO, root, MPI_COMM_WORLD);
  for (i=0;i<vSize;++i) { 
    rBuf[i] = 3*(rBuf[i]);
//    printf(__FILE__ ": process %i computes  %f \n",world_rank, rBuf[i]);
  }
  AMPI_Gatherv(rBuf, vSize, TYPE_MACRO, sBuf, counts, displs, TYPE_MACRO, root, MPI_COMM_WORLD);
  if (world_rank == root) {
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	y+=sBuf[r*vSize+i];
      }
    }
//    printf(__FILE__ ": process 0 result is %f \n",y);
    delete[] sBuf; delete[] counts; delete[] displs;
    *out=y;
  } 
  delete[] rBuf;
  return 0;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y;
  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.5;
    x<<=xp;
    head(&x,&y);
    y>>=yp;
    printf(__FILE__ ": process 0 got number %f \n", yp);
  } 
  else {
    head(&x,&y);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,1,1,&xp,&yp);
    w=1.0;
    fos_reverse(world_rank,1,1,&w,&g);
    printf(__FILE__ ": process 0 got gradient %f \n", g);
  } 
  else {
    tape_doc(world_rank,0,0,0,0);
    fos_reverse(world_rank,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return 0;
}
