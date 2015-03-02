#include <cstdio>
#include <cmath>
#include "ampi/ampi.h"
#include "adolc/adolc.h"

void head(adouble* in, adouble *out) { 
  int world_rank; 
  const int vSize=3;
  const int root=0;
  adouble x=*in,y=0.0;
  int gSize,i,r;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  AMPI_Comm_size(MPI_COMM_WORLD, &gSize);
  adouble *buf1=NULL;
  ensureContiguousLocations(vSize);
  adouble *buf2=new adouble[vSize]; 
  ensureContiguousLocations(vSize*gSize);
  adouble *buf3=new adouble[vSize*gSize]; 
  ensureContiguousLocations(vSize*gSize);
  adouble *buf4=new adouble[vSize*gSize];
  int *counts=new int[gSize];
  int *displs=new int[gSize];
  if (world_rank == root) {
    ensureContiguousLocations(vSize*gSize);
    buf1=new adouble[vSize*gSize];
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	buf1[r*vSize+i] = x*(i+1)*(r+1);
      }
    }
  }
  for(r=0;r<gSize;++r) { 
    for (i=0;i<vSize;++i) { 
      counts[r]=vSize;
      displs[r]=r*vSize;
    }
  }
  AMPI_Scatterv(buf1, counts,displs, TYPE_MACRO, buf2, vSize, TYPE_MACRO, root, MPI_COMM_WORLD);
  for (i=0;i<vSize;++i) { 
    buf2[i]*=3.0;
  }
  AMPI_Allgatherv(buf2, vSize, TYPE_MACRO, buf3, counts, displs, TYPE_MACRO, MPI_COMM_WORLD);
  for(r=0;r<gSize;++r) { 
    for (i=0;i<vSize;++i) { 
      buf3[r*vSize+i]*=(world_rank+1);
    }
  }
  AMPI_Reduce(buf3,
	      buf4,
	      vSize*gSize,
	      TYPE_MACRO,
	      MPI_SUM,
	      root,
	      MPI_COMM_WORLD);
  if (world_rank == root) {
    for(r=0;r<gSize;++r) { 
      for (i=0;i<vSize;++i) { 
	y+=buf4[r*vSize+i];
      }
    }
    delete [] buf1;
    *out=y;
  } 
  delete [] counts; delete [] displs;
  delete [] buf2;
  delete [] buf3;
  delete [] buf4;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  adouble x,y;
  double xp,yp,w, g ;
  trace_on(world_rank,1);
  if (world_rank == 0) {
    xp=3.14;
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
