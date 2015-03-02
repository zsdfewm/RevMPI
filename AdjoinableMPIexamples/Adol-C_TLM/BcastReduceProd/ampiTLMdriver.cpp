#include <cstdio>
#include <cmath>
#include <limits>
#include "ampi/ampi.h"
#include "adolc/adolc.h"
#include "ampiHead.h"

int main(int argc, char** argv) {
  int rc;
  int world_rank;
  int dirs=3;
  int order=4;
  int n=1;
  int m=1;
  double epsilon=1e-12;
  adouble *x=new adouble[n],*y=new adouble[m];
  double *xp=myalloc1(n),*yp=myalloc1(m);
  double *x_fos=myalloc1(n), *y_fos=myalloc1(m);
  double **x_fov=myalloc2(n,dirs), **y_fov=myalloc2(m,dirs);
  double **x_hos=myalloc2(n,order), **y_hos=myalloc2(m,order) ;
  double ***x_hov=myalloc3(n,dirs,order), ***y_hov=myalloc3(m,dirs,order) ;

  AMPI_Init_NT(0,0);
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  for (int i=0;i<n;++i) xp[i]=3.5*(i+1);
  trace_on(world_rank,1);
  if (world_rank == 0) {
    for (int i =0;i<n;++i) x[i]<<=xp[i];
    head(x,y);
    for (int j =0;j<m;++j) y[j]>>=yp[j];
    for (int j =0;j<m;++j) printf(__FILE__ ": process 0 got number %f \n", yp[j]);
  } 
  else {
    head(x,y);
  } 
  trace_off(1);
  if (world_rank == 0) {
    tape_doc(world_rank,m,n,xp,yp);
    zos_forward(world_rank,m,n,0,xp,yp);
    for (int j =0;j<m;++j)  {
      if (epsilon<fabs(y[j].getValue()-yp[j])) { 
	printf(__FILE__ ": y - zos inconsistency |%e - %e|=%e > epsilon %e\n",y[j].getValue(),yp[j],fabs(y[j].getValue()-yp[j]),epsilon);
	rc++;
      }
    }
    for (int i =0;i<n;++i) {
      for (int d=0;d<dirs;++d)  { 
	x_fov[i][d]=1.0*(d+1);
      }
    }
    fov_forward(world_rank,m,n,dirs,xp,x_fov,yp,y_fov);
    for (int j =0;j<m;++j)  {
      if (epsilon<fabs(y[j].getValue()-yp[j])) { 
	printf(__FILE__ ": y - fov inconsistency |%e - %e|=%e > epsilon %e\n",y[j].getValue(),yp[j],fabs(y[j].getValue()-yp[j]),epsilon);
	rc++;
      }
    }
    for (int j =0;j<m;++j) printf(__FILE__ ": process 0 got gradient %e \n", y_fov[j][0]);
    for (int d=0;d<dirs;++d)  { 
      for (int i=0;i<n;++i) { 
	x_fos[i]=x_fov[i][d];
      }
      fos_forward(world_rank,m,n,0,xp,x_fos,yp,y_fos);
      for (int j =0;j<m;++j)  {
	if (epsilon<fabs(y[j].getValue()-yp[j])) { 
	  printf(__FILE__ ": y - fos inconsistency |%e - %e|=%e > epsilon %e\n",y[j].getValue(),yp[j],fabs(y[j].getValue()-yp[j]),epsilon);
	  rc++;
	}
	if (epsilon<fabs(y_fos[j]-y_fov[j][d])) { 
	  printf(__FILE__ ": fos - fov inconsistency |%e - %e|=%e > epsilon %e\n",y_fos[j],y_fov[j][d], fabs(y_fos[j]-y_fov[j][d]),epsilon);
	  rc++;
	}
      }
    }
    for(int i=0;i<n;++i) { 
      for(int d=0;d<dirs;++d) { 
	x_hov[i][d][0]=1.0*(d+1);
	for (int o=1;o<order;++o) { 
	  x_hov[i][d][o]=0.0;
	}
      }
    }
    hov_forward(world_rank,m,n,order,dirs,xp,x_hov,yp,y_hov);
    for(int d=0;d<dirs;++d) { 
      for (int i =0;i<n;++i) {
	for (int o=0;o<order;++o)  { 
	  x_hos[i][o]=x_hov[i][d][o];
	}
      }
      hos_forward(world_rank,m,n,order,0,xp,x_hos,yp,y_hos);
      for (int j =0;j<m;++j)  {
	if (epsilon<fabs(y[j].getValue()-yp[j])) { 
	  printf(__FILE__ ": y - hos inconsistency |%e - %e|=%e > epsilon %e\n",y[j].getValue(),yp[j],fabs(y[j].getValue()-yp[j]),epsilon);
	  rc++;
	}
	if (epsilon<fabs(y_fov[j][d]-y_hov[j][d][0])) { 
	  printf(__FILE__ ": fov - hov inconsistency |%e - %e|=%e > epsilon %e\n",y_fov[j][d],y_hov[j][d][0], fabs(y_fov[j][d]-y_hov[j][d][0]),epsilon);
	  rc++;
	}
	for (int o=0;o<order;++o)  { 
	  if (epsilon<fabs(y_hos[j][o]-y_hov[j][d][o])) { 
	    printf(__FILE__ ": hos - hov inconsistency |%e - %e|=%e > epsilon %e\n",y_hos[j][o],y_hov[j][d][o], fabs(y_hos[j][o]-y_hov[j][d][o]),epsilon);
	    rc++;
	  }
	}
      }
    }
  } 
  else {
    tape_doc(world_rank,0,0,0,0);
    zos_forward(world_rank,0,0,0,0,0);
    fov_forward(world_rank,0,0,dirs,0,0,0,0);
    for (int d=0;d<dirs;++d) fos_forward(world_rank,0,0,0,0,0,0,0);
    hov_forward(world_rank,0,0,order,dirs,0,0,0,0);
    for (int d=0;d<dirs;++d) hos_forward(world_rank,0,0,order,0,0,0,0,0);
  }   
  AMPI_Finalize_NT();
  return rc;
}
