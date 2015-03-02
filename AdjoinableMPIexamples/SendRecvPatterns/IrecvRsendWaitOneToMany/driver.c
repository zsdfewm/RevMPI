#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
  MPI_Init(0,0);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  if (world_rank != 0) { /* asking questions to rank 0 */
    double answer; 
    int dummyQuestion;
    MPI_Request req; 
    /* post the recv to make sure it is there when rank 0 does the rsend */
    MPI_Irecv(&answer, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &req);
    /* now ask the question by sending a zero length message */
    MPI_Send(&dummyQuestion, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
    printf(__FILE__ ": process %d received answer %f \n", world_rank, answer);
  } else { /* rank 0 handing out answers needs to be quick */
    int i=1;
    while (i<world_size) { 
      int dummyQuestion; 
      double answer=1.0*i;
      MPI_Status s; 
      MPI_Recv(&dummyQuestion, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
      /* to be quick we can use rsend because we know the other side has posted the recv */
      MPI_Rsend(&answer, 1, MPI_DOUBLE, s.MPI_SOURCE, 0, MPI_COMM_WORLD);
      printf(__FILE__ ": sent answer %f to process %d \n", answer,s.MPI_SOURCE);
      ++i;
    } 
  }
  MPI_Finalize();
  return 0;
}
