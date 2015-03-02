/* Program ported to AMPI. Porting should be made by the user
 * in general (e.g. for pairedWith), but could be automated sometimes.
 * Should behave identical to program.c
 * MPI_Reduce with a user-defined reduction function */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ampi/ampi.h"

typedef struct {
  double  array[3];
  int     middle ;
  float   right;
} OurType ;

void oursum (void *invec, void *inoutvec,
             int *len,
             MPI_Datatype *datatype) {
  int i ;
  int world_rank;
  OurType *added, *recipient ;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  for(i=0 ; i<*len; ++i) {
    added     = ((OurType*)invec)+i ;
    recipient = ((OurType*)inoutvec)+i ;
/*     printf(__FILE__ "   > process %i, index:%i, ([%f,%f,%f];%i;%f) added into ([%f,%f,%f];%i;%f)\n", */
/*            world_rank,i, */
/*            added->array[0],added->array[1],added->array[2],added->middle,added->right, */
/*            recipient->array[0],recipient->array[1],recipient->array[2],recipient->middle,recipient->right) ; */
    recipient->array[0] += added->array[0] ;
    recipient->array[1] += added->array[1] ;
    recipient->array[2] *= added->array[2] ;
    recipient->middle += added->middle ;
    recipient->right += added->right ;
  }
}

void head(OurType *x, OurType *y, int len, MPI_Datatype mpi_OurType, MPI_Comm comm) {
  MPI_Op op_oursum ;
  int i ;
  AMPI_Op_create_NT(oursum, 1, &op_oursum) ;
  AMPI_Reduce(x, y, len, mpi_OurType, op_oursum, 0, comm);
  for (i=0 ; i<len ; ++i) {
    y[i].array[1] = y[i].array[1] * y[i].array[1] ;
  }
}

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  OurType *x, *y ;
  MPI_Datatype mpi_OurType ;
  int blockLengths[3] = {3,1,1} ;
  MPI_Datatype blockTypes[3] = {MPI_DOUBLE, MPI_INT, MPI_FLOAT} ;
  MPI_Aint blockOffsets[3] ;
  int i ;
  int len = 3 ;
  x = (OurType*) malloc(len*sizeof(OurType)) ;
  y = (OurType*) malloc(len*sizeof(OurType)) ;
  blockOffsets[0] = (char*)&(x[0].array ) - (char*)&x[0];
  blockOffsets[1] = (char*)&(x[0].middle) - (char*)&x[0];
  blockOffsets[2] = (char*)&(x[0].right ) - (char*)&x[0];
  AMPI_Type_create_struct_NT(3, blockLengths, blockOffsets, blockTypes, &mpi_OurType) ;
  MPI_Type_commit(&mpi_OurType) ;
  for (i=0 ; i<len ; ++i) {
    y[i].array[0] = 9.0 ;
    y[i].array[1] = 9.0 ;
    y[i].array[2] = 9.0 ;
    y[i].middle = 9 ;
    y[i].right = 9 ;
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
  }
  if (world_rank==0) {
    printf(__FILE__ ": process 0 (root) initial x[2]: [%5.2f,%5.2f,%5.2f]; %i;%5.2f\n",
           x[2].array[0],x[2].array[1],x[2].array[2],x[2].middle,x[2].right) ;
  }
  if (world_rank==1) {
    printf(__FILE__ ": process 1 (std)  initial x[2]: [%5.2f,%5.2f,%5.2f]; %i;%5.2f\n",
           x[2].array[0],x[2].array[1],x[2].array[2],x[2].middle,x[2].right) ;
  }

  head(x,y,len,mpi_OurType,MPI_COMM_WORLD) ;

  if (world_rank==0) {
    printf(__FILE__ ": process 0 (root) gets reduced[2]:[%5.2f,%5.2f,%5.2f]; %i;%5.2f (should be [ 8.00, 4.00, 3.50]; 5; 9.20)\n",
           y[2].array[0],y[2].array[1],y[2].array[2],y[2].middle,y[2].right) ;
  }
  if (world_rank==1) {
    printf(__FILE__ ": process 1 (std)  gets reduced[2]:[%5.2f,%5.2f,%5.2f]; %i;%5.2f (should be [ 9.00,81.00, 9.00]; 9; 9.00)\n",
           y[2].array[0],y[2].array[1],y[2].array[2],y[2].middle,y[2].right) ;
  }

  AMPI_Finalize_NT();
  return 0;
}
