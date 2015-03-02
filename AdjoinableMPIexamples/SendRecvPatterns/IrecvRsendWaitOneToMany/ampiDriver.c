#include <stdio.h>
#include "ampi/ampi.h"

int main(int argc, char** argv) {
  AMPI_Init_NT(0,0);
  int world_size;
  AMPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  AMPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank != 0) { /* asking questions to rank 0 */
    double answer; 
    int dummyQuestion;
    AMPI_Request req; 
    /* post the recv to make sure it is there when rank 0 does the rsend */
    AMPI_Irecv(&answer, 1, TYPE_MACRO, 0, 0, AMPI_FROM_RSEND, MPI_COMM_WORLD, &req); /* adjoint needs wait / nullify */
    /* now ask the question by sending a zero length message */
    AMPI_Send(&dummyQuestion, 0, MPI_INT, 0, 0, AMPI_TO_RECV, MPI_COMM_WORLD); /* adjoint does nothing */
    AMPI_Wait_ST(&req, &answer, MPI_STATUS_IGNORE); /* adjoint needs to isend */
    printf(__FILE__ ": process %d received answer %f \n", world_rank, answer);
  } else { /* rank 0 handing out answers needs to be quick */
    int i=1;
    while (i<world_size) { 
      int dummyQuestion; 
      double answer=1.0*i;
      MPI_Status s; 
      AMPI_Recv(&dummyQuestion, 0, MPI_INT,  MPI_ANY_SOURCE, MPI_ANY_TAG, AMPI_FROM_SEND, MPI_COMM_WORLD, &s); /* adjoint does nothing */
      /* to be quick we can use rsend because we know the other side has posted the recv */
      AMPI_Rsend(&answer, 1, TYPE_MACRO, s.MPI_SOURCE, 0, AMPI_TO_IRECV_WAIT, MPI_COMM_WORLD); /* adjoint needs to recv / increment */
      printf(__FILE__ ": sent answer %f to process %d \n", answer,s.MPI_SOURCE);
      ++i;
    } 
  }
  AMPI_Finalize_NT();
  return 0;
}
