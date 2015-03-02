/* Desired tapenade adjoint using the AMPI library
 * MPI_Reduce with a user-defined reduction function */

#include <stdio.h>
#include <stdlib.h>
#include "ampi/ampi.h"
#include "ampi/libCommon/modified.h"
#include "ampi/adTool/support.h"
#include "ADFirstAidKit/adBuffer.h"


typedef struct {
  double  array[3];
  int     middle ;
  float   right;
} OurType ;

typedef struct {
  double  array[3];
} OurType_b ;

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
/*     if (i==2) */
/*       printf(__FILE__ "   > process %i, index %i: ([%f,%f,%f];%i;%f) => ([%f,%f,%f];%i;%f)\n",world_rank,i, */
/*              added->array[0],added->array[1],added->array[2],added->middle,added->right, */
/*              recipient->array[0],recipient->array[1],recipient->array[2],recipient->middle,recipient->right) ; */
    recipient->array[0] += added->array[0] ;
    recipient->array[1] += added->array[1] ;
    recipient->array[2] *= added->array[2] ;
    recipient->middle += added->middle ;
    recipient->right += added->right ;
/*     if (i==2) */
/*       printf(__FILE__ "   > process %i, index %i, result: => ([%f,%f,%f];%i;%f)\n",world_rank,i, */
/*            recipient->array[0],recipient->array[1],recipient->array[2],recipient->middle,recipient->right) ; */
  }
}

void oursum_b(void *invec, void *invecb, void *inoutvec,void *inoutvecb,
            int *len, MPI_Datatype *datatype, MPI_Datatype *datatypeb) {
  int i ;
  int world_rank;
  OurType *added, *recipient ;
  OurType_b *addedb, *recipientb ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
/*   printf(__FILE__ ": process %i calls oursum_b with len %i \n",world_rank,*len) ; */
  for(i=0 ; i<*len; ++i) {
    added     = ((OurType*)invec)+i ;
    recipient = ((OurType*)inoutvec)+i ;
    addedb     = ((OurType_b*)invecb)+i ;
    recipientb = ((OurType_b*)inoutvecb)+i ;
/*     if (i==2) */
/*       printf(__FILE__ "   > process %i, index %i: ([%f,%f,%f]) => ([%f,%f,%f])\n",world_rank,i, */
/*            added->array[0],added->array[1],added->array[2], */
/*            recipient->array[0],recipient->array[1],recipient->array[2]) ; */
/*     if (i==2) */
/*       printf(__FILE__ "   Dbefore pr%i, index %i: ([%f,%f,%f]) => ([%f,%f,%f])\n",world_rank,i, */
/*            addedb->array[0],addedb->array[1],addedb->array[2], */
/*            recipientb->array[0],recipientb->array[1],recipientb->array[2]) ; */
    addedb->array[0] += recipientb->array[0] ;
    addedb->array[1] += recipientb->array[1] ;
    addedb->array[2] += recipientb->array[2]*recipient->array[2] ;
    recipientb->array[2] *= added->array[2] ;
/*     if (i==2) */
/*       printf(__FILE__ "   D after pr%i, index %i: ([%f,%f,%f]) => ([%f,%f,%f])\n",world_rank,i, */
/*            addedb->array[0],addedb->array[1],addedb->array[2], */
/*            recipientb->array[0],recipientb->array[1],recipientb->array[2]) ; */
  }
}

void head_b(OurType *x, OurType_b *xb, OurType *y, OurType_b *yb, int len,
            MPI_Datatype mpi_OurType, MPI_Datatype mpi_OurType_b, MPI_Comm comm) {
  int i;
  MPI_Op op_oursum ;
  AMPI_Op_create_NT(oursum, 1/*1 means commutative*/, &op_oursum) ;
  FWS_AMPI_Reduce(x, y, len, mpi_OurType, op_oursum, 0, comm);
  /*  pushreal8(y[i].array[1]); */
  /*  y[i].array[1] = y[i].array[1] * y[i].array[1] ; */
  /*  popreal8(&(y[i].array[1])); */
  ADTOOL_AMPI_Turn(x, xb) ;
  ADTOOL_AMPI_Turn(y, yb) ;
  for (i=0 ; i<len ; ++i) {
    yb[i].array[1] = 2.0 * y[i].array[1] * yb[i].array[1] ;
  }
  /* implement the "pedestrian way" and reverse it */
  BWS_AMPI_Reduce(x, xb, y, yb, len, mpi_OurType, mpi_OurType_b, op_oursum, &oursum_b, 0, comm);
  MPI_Op_free(&op_oursum) ;
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  OurType *x, *y ;
  OurType_b *xb,*yb ;
  MPI_Datatype mpi_OurType ;
  int blockLengths[3] = {3,1,1} ;
  MPI_Datatype blockTypes[3] = {MPI_DOUBLE, MPI_INT, MPI_FLOAT} ;
  MPI_Aint blockOffsets[3] ;
  int i ;
  int len = 3 ;
  x = (OurType*) malloc(len*sizeof(OurType)) ;
  y = (OurType*) malloc(len*sizeof(OurType)) ;
  xb = (OurType_b*) malloc(len*sizeof(OurType_b)) ;
  yb = (OurType_b*) malloc(len*sizeof(OurType_b)) ;
  blockOffsets[0] = (char*)&(x[0].array ) - (char*)&x[0];
  blockOffsets[1] = (char*)&(x[0].middle) - (char*)&x[0];
  blockOffsets[2] = (char*)&(x[0].right ) - (char*)&x[0];
  AMPI_Type_create_struct_NT(3, blockLengths, blockOffsets, blockTypes, &mpi_OurType) ;
  MPI_Type_commit(&mpi_OurType) ;
  MPI_Datatype mpi_OurType_b ;
  int blockLengthsb[1] = {3} ;
  MPI_Datatype blockTypesb[1] = {MPI_DOUBLE} ;
  MPI_Aint blockOffsetsb[1] ;
  blockOffsetsb[0] = (char*)&(xb[0].array ) - (char*)&xb[0];
  AMPI_Type_create_struct_NT(1, blockLengthsb, blockOffsetsb, blockTypesb, &mpi_OurType_b) ;
  MPI_Type_commit(&mpi_OurType_b) ;
  for (i=0 ; i<len ; ++i) {
    y[i].array[0] = 9.0 ;
    y[i].array[1] = 9.0 ;
    y[i].array[2] = 9.0 ;
    y[i].middle = 9 ;
    y[i].right = 9 ;

    yb[i].array[0] = 1.0 ;
    yb[i].array[1] = 1.0 ;
    yb[i].array[2] = 1.0 ;

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

    xb[i].array[0] = 1.0 ;
    xb[i].array[1] = 1.0 ;
    xb[i].array[2] = 1.0 ;
  }
/*   if (world_rank==0) { */
/*     printf(__FILE__ ": process 0 (root) initial x[2]: [%5.2f,%5.2f,%5.2f]; %i;%5.2f\n", */
/*            x[2].array[0],x[2].array[1],x[2].array[2],x[2].middle,x[2].right) ; */
/*     printf(__FILE__ ": process 0 (root) initial yb[2]:[%5.2f,%5.2f,%5.2f]\n", */
/*            yb[2].array[0],yb[2].array[1],yb[2].array[2]) ; */
/*   } */
/*   if (world_rank==1) { */
/*     printf(__FILE__ ": process 1 (std)  initial x[2]: [%5.2f,%5.2f,%5.2f]; %i;%5.2f\n", */
/*            x[2].array[0],x[2].array[1],x[2].array[2],x[2].middle,x[2].right) ; */
/*     printf(__FILE__ ": process 1 (std)  initial yb[2]:[%5.2f,%5.2f,%5.2f]\n", */
/*            yb[2].array[0],yb[2].array[1],yb[2].array[2]) ; */
/*   } */

  head_b(x,xb,y,yb,len,mpi_OurType,mpi_OurType_b,MPI_COMM_WORLD) ;

/*   if (world_rank==0) { */
/*     printf(__FILE__ ": process 0 (root) gets xb[2]:[%5.2f,%5.2f,%5.2f]          (should be ?)\n", */
/*            xb[2].array[0],xb[2].array[1],xb[2].array[2]) ; */
/*     printf(__FILE__ ": process 0 (root) gets yb[2]:[%5.2f,%5.2f,%5.2f]          (should be ?)\n", */
/*            yb[2].array[0],yb[2].array[1],yb[2].array[2]) ; */
/*   } */
/*   if (world_rank==1) { */
/*     printf(__FILE__ ": process 1 (std)  gets xb[2]:[%5.2f,%5.2f,%5.2f]          (should be ?)\n", */
/*            xb[2].array[0],xb[2].array[1],xb[2].array[2]) ; */
/*     printf(__FILE__ ": process 1 (std)  gets yb[2]:[%5.2f,%5.2f,%5.2f]          (should be ?)\n", */
/*            yb[2].array[0],yb[2].array[1],yb[2].array[2]) ; */
/*   } */

  double sumb = 0.0 ;
  int j ;
  for (i=0 ; i<len ; ++i) {
    for (j=0 ; j<3 ; ++j) {
      sumb += xb[i].array[j] + yb[i].array[j] ;
    }
  }
  double globalSumb = 0.0 ;
  MPI_Reduce(&sumb,&globalSumb,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD) ;
  if (world_rank==0)
    printf(__FILE__ ": process %i, sum of all  input adjoints:%f  (should be 310.506 on 4 procs)\n",world_rank,globalSumb) ;

  AMPI_Finalize_NT();
  return 0;
}
