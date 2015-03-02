/* Desired tapenade tangent using the AMPI library.
 * MPI_Reduce with a user-defined reduction function */

#include <stdio.h>
#include <stdlib.h>
#include "ampi/ampi.h"
#include "ampi/libCommon/modified.h"
#include "ampi/adTool/support.h"


typedef struct {
  double  array[3];
  int     middle ;
  float   right;
} OurType ;

typedef struct {
  double  array[3];
} OurType_d ;

void oursum(void *invec, void *inoutvec,
            int *len,
            MPI_Datatype *datatype) {
  int i ;
  int world_rank;
  OurType *added, *recipient ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
/*   printf(__FILE__ ": process %i calls oursum with len %i \n",world_rank,*len) ; */
  for(i=0 ; i<*len; ++i) {
    added     = ((OurType*)invec)+i ;
    recipient = ((OurType*)inoutvec)+i ;
/*     printf(__FILE__ "   > process %i, index i: %i, ([%f,%f,%f];%i;%f) => ([%f,%f,%f];%i;%f)\n",world_rank,i, */
/*            added->array[0],added->array[1],added->array[2],added->middle,added->right, */
/*            recipient->array[0],recipient->array[1],recipient->array[2],recipient->middle,recipient->right) ; */
    recipient->array[0] += added->array[0] ;
    recipient->array[1] += added->array[1] ;
    recipient->array[2] *= added->array[2] ;
    recipient->middle += added->middle ;
    recipient->right += added->right ;
  }
}

void oursum_d(void *invec, void *invecd, void *inoutvec, void *inoutvecd,
              int *len,
              MPI_Datatype *datatype, MPI_Datatype *datatyped) {
  int i ;
  int world_rank;
  OurType *added, *recipient ;
  OurType_d *addedd, *recipientd ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
/*   printf(__FILE__ ": process %i calls oursum_d with len %i \n",world_rank,*len) ; */
  for(i=0 ; i<*len; ++i) {
    added     = ((OurType*)invec)+i ;
    recipient = ((OurType*)inoutvec)+i ;
    addedd     = ((OurType_d*)invecd)+i ;
    recipientd = ((OurType_d*)inoutvecd)+i ;
/*     printf(__FILE__ "   V process %i, index %i, ([%f,%f,%f];%i;%f) => ([%f,%f,%f];%i;%f)\n", world_rank, i, */
/*            added->array[0],added->array[1],added->array[2],added->middle,added->right, */
/*            recipient->array[0],recipient->array[1],recipient->array[2],recipient->middle,recipient->right) ; */
/*     printf(__FILE__ "   D process %i, index %i, ([%f,%f,%f]) => ([%f,%f,%f])\n", world_rank, i, */
/*            addedd->array[0],addedd->array[1],addedd->array[2], */
/*            recipientd->array[0],recipientd->array[1],recipientd->array[2]) ; */
    recipientd->array[0] += addedd->array[0] ;
    recipient->array[0] += added->array[0] ;
    recipientd->array[1] += addedd->array[1] ;
    recipient->array[1] += added->array[1] ;
    recipientd->array[2] =
      recipientd->array[2]*added->array[2] + recipient->array[2]*addedd->array[2];
    recipient->array[2] *= added->array[2] ;
    recipient->middle += added->middle ;
    recipient->right += added->right ;
/*     printf(__FILE__ "   NV process %i, index %i, ([%f,%f,%f];%i;%f) => ([%f,%f,%f];%i;%f)\n", world_rank, i, */
/*            added->array[0],added->array[1],added->array[2],added->middle,added->right, */
/*            recipient->array[0],recipient->array[1],recipient->array[2],recipient->middle,recipient->right) ; */
/*     printf(__FILE__ "   ND process %i, index %i, ([%f,%f,%f]) => ([%f,%f,%f])\n", world_rank, i, */
/*            addedd->array[0],addedd->array[1],addedd->array[2], */
/*            recipientd->array[0],recipientd->array[1],recipientd->array[2]) ; */
  }
}

void head_d(OurType *x, OurType_d *xd, OurType *y, OurType_d *yd, int len,
            MPI_Datatype mpi_OurType, MPI_Datatype mpi_OurType_d, MPI_Comm comm) {
  int i, rc;
  MPI_Op op_oursum ;
  AMPI_Op_create_NT(oursum, 1, &op_oursum) ;

  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  rc = TLS_AMPI_Reduce(x, xd, y, yd, len, mpi_OurType, mpi_OurType_d, op_oursum, &oursum_d, 0, comm);
  for (i=0 ; i<len ; ++i) {
    yd[i].array[1] = 2.0*y[i].array[1]*yd[i].array[1] ;
    y[i].array[1] = y[i].array[1] * y[i].array[1] ;
  }
  AMPI_Op_free_NT(&op_oursum) ;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  OurType *x, *y ;
  OurType_d *xd,*yd ;
  MPI_Datatype mpi_OurType ;
  int blockLengths[3] = {3,1,1} ;
  MPI_Datatype blockTypes[3] = {MPI_DOUBLE, MPI_INT, MPI_FLOAT} ;
  MPI_Aint blockOffsets[3] ;
  int i ;
  int len = 3 ;
  x = (OurType*) malloc(len*sizeof(OurType)) ;
  y = (OurType*) malloc(len*sizeof(OurType)) ;
  xd = (OurType_d*) malloc(len*sizeof(OurType_d)) ;
  yd = (OurType_d*) malloc(len*sizeof(OurType_d)) ;
  blockOffsets[0] = (char*)&(x[0].array ) - (char*)&x[0];
  blockOffsets[1] = (char*)&(x[0].middle) - (char*)&x[0];
  blockOffsets[2] = (char*)&(x[0].right ) - (char*)&x[0];
  AMPI_Type_create_struct_NT(3, blockLengths, blockOffsets, blockTypes, &mpi_OurType) ;
  MPI_Type_commit(&mpi_OurType) ;
  MPI_Datatype mpi_OurType_d ;
  int blockLengthsd[1] = {3} ;
  MPI_Datatype blockTypesd[1] = {MPI_DOUBLE} ;
  MPI_Aint blockOffsetsd[1] ;
  blockOffsetsd[0] = (char*)&(xd[0].array ) - (char*)&xd[0];
  AMPI_Type_create_struct_NT(1, blockLengthsd, blockOffsetsd, blockTypesd, &mpi_OurType_d) ;
  MPI_Type_commit(&mpi_OurType_d) ;
  for (i=0 ; i<len ; ++i) {
    y[i].array[0] = 9.0 ;
    y[i].array[1] = 9.0 ;
    y[i].array[2] = 9.0 ;
    y[i].middle = 9 ;
    y[i].right = 9 ;

    yd[i].array[0] = 1.0 ;
    yd[i].array[1] = 1.0 ;
    yd[i].array[2] = 1.0 ;

    if (world_rank == 0) {
      x[i].array[0] = 0.5 ;
      x[i].array[1] = 0.5 ;
      x[i].array[2] = 0.8 ;
      x[i].middle = -1 ;
      x[i].right = 2.3 ;
    } else {
      x[i].array[0] = world_rank+0.5 ;
      x[i].array[1] = 0.5 ;
      x[i].array[2] = 0.9*world_rank ;
      x[i].middle = world_rank ;
      x[i].right = 2.3 ;
    }

    xd[i].array[0] = 1.0 ;
    xd[i].array[1] = 1.0 ;
    xd[i].array[2] = 1.0 ;
  }
/*   if (world_rank==0) { */
/*     printf(__FILE__ ": process 0 (root) initial x[2]: [%5.2f,%5.2f,%5.2f]; %i;%5.2f\n", */
/*            x[2].array[0],x[2].array[1],x[2].array[2],x[2].middle,x[2].right) ; */
/*     printf(__FILE__ ": process 0 (root) initial xd[2]:[%5.2f,%5.2f,%5.2f]\n", */
/*            xd[2].array[0],xd[2].array[1],xd[2].array[2]) ; */
/*   } */
/*   if (world_rank==1) { */
/*     printf(__FILE__ ": process 1 (std)  initial x[2]: [%5.2f,%5.2f,%5.2f]; %i;%5.2f\n", */
/*            x[2].array[0],x[2].array[1],x[2].array[2],x[2].middle,x[2].right) ; */
/*     printf(__FILE__ ": process 1 (std)  initial xd[2]:[%5.2f,%5.2f,%5.2f]\n", */
/*            xd[2].array[0],xd[2].array[1],xd[2].array[2]) ; */
/*   } */

  head_d(x,xd,y,yd,len,mpi_OurType,mpi_OurType_d,MPI_COMM_WORLD) ;

/*   if (world_rank==0) { */
/*     printf(__FILE__ ": process 0 (root) gets reduced[2]:[%5.2f,%5.2f,%5.2f]; %i;%5.2f (should be (4procs) [ 8.00, 4.00, 3.50]; 5; 9.20)\n", */
/*            y[2].array[0],y[2].array[1],y[2].array[2],y[2].middle,y[2].right) ; */
/*     printf(__FILE__ ": process 0 (root) gets tangent[2]:[%5.2f,%5.2f,%5.2f]          (should be (4procs) [ 4.00,16.00,11.50])\n", */
/*            yd[2].array[0],yd[2].array[1],yd[2].array[2]) ; */
/*   } */
/*   if (world_rank==1) { */
/*     printf(__FILE__ ": process 1 (std)  gets reduced[2]:[%5.2f,%5.2f,%5.2f]; %i;%5.2f (should be (4procs) [ 9.00,81.00, 9.00]; 9; 9.00)\n", */
/*            y[2].array[0],y[2].array[1],y[2].array[2],y[2].middle,y[2].right) ; */
/*     printf(__FILE__ ": process 1 (std)  gets tangent[2]:[%5.2f,%5.2f,%5.2f]          (should be (4procs) [ 1.00,18.00, 1.00])\n", */
/*            yd[2].array[0],yd[2].array[1],yd[2].array[2]) ; */
/*   } */

  double sumd = 0.0 ;
  int j ;
  for (i=0 ; i<len ; ++i) {
    for (j=0 ; j<3 ; ++j) {
      sumd += xd[i].array[j] + yd[i].array[j] ;
    }
  }
  double globalSumd = 0.0 ;
  MPI_Reduce(&sumd,&globalSumd,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD) ;
  if (world_rank==0)
    printf(__FILE__ ": process %i, sum of all output tangents:%f  (should be 310.506 on 4 procs)\n",world_rank,globalSumd) ;

  AMPI_Finalize_NT();
  return 0;
}
