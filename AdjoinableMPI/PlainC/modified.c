/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>
#include "ampi/userIF/modified.h"

MPI_Datatype AMPI_ADOUBLE_PRECISION;
MPI_Datatype AMPI_AREAL;


int AMPI_Init(int* argc, 
	      char*** argv) { 
  return MPI_Init(argc,argv);
}

int AMPI_Finalize(int* argc, 
		  char*** argv) { 
  return MPI_Finalize();
}

int AMPI_Buffer_attach(void *buffer, 
		       int size) { 
  return MPI_Buffer_attach(buffer,
			   size);

}

int AMPI_Buffer_detach(void *buffer, 
		       int *size){ 
  return MPI_Buffer_detach(buffer,
			   size);
}


int AMPI_Send(void* buf, 
	      int count, 
	      MPI_Datatype datatype, 
	      int dest, 
	      int tag,
	      AMPI_PairedWith pairedWith,
	      MPI_Comm comm) {
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	||
	pairedWith==AMPI_TO_IRECV_WAITALL
	)) MPI_Abort(comm, MPI_ERR_ARG);
  /* [llh] TODO: pass (pairedWith;AMPI_FROM_SEND) for verification on the receiver side */
  return MPI_Send(buf,
		  count,
		  datatype,
		  dest,
		  tag,
		  comm);
}

int AMPI_Recv(void* buf, 
	      int count,
	      MPI_Datatype datatype, 
	      int src, 
	      int tag,
	      AMPI_PairedWith pairedWith,
	      MPI_Comm comm,
	      MPI_Status* status) { 
  /* [llh] TODO: verify with the (pairedWith;AMPI_FROM_SEND) passed by the sender side */
  if (!(
	pairedWith==AMPI_FROM_SEND 
	|| 
	pairedWith==AMPI_FROM_BSEND 
	||
	pairedWith==AMPI_FROM_RSEND 
	||
	pairedWith==AMPI_FROM_ISEND_WAIT
	||
	pairedWith==AMPI_FROM_ISEND_WAITALL
	)) MPI_Abort(comm, MPI_ERR_ARG);
  return MPI_Recv(buf,
		  count,
		  datatype,
		  src,
		  tag,
		  comm,
		  status);
}  

int AMPI_Isend (void* buf, 
		int count, 
		MPI_Datatype datatype, 
		int dest, 
		int tag,
		AMPI_PairedWith pairedWith,
		MPI_Comm comm, 
		AMPI_Request* request) { 
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	||
	pairedWith==AMPI_TO_IRECV_WAITALL
	)) MPI_Abort(comm, MPI_ERR_ARG);
  return MPI_Isend(buf,
		   count,
		   datatype,
		   dest,
		   tag,
		   comm,
#ifdef AMPI_FORTRANCOMPATIBLE
		   request
#else 
		   &(request->plainRequest)
#endif 
		   );
}

int AMPI_Irecv (void* buf, 
		int count, 
		MPI_Datatype datatype, 
		int src, 
		int tag,
		AMPI_PairedWith pairedWith,
		MPI_Comm comm, 
		AMPI_Request* request) { 
  if (!(
	pairedWith==AMPI_FROM_SEND 
	|| 
	pairedWith==AMPI_FROM_BSEND 
	||
	pairedWith==AMPI_FROM_RSEND 
	||
	pairedWith==AMPI_FROM_ISEND_WAIT
	||
	pairedWith==AMPI_FROM_ISEND_WAITALL
	)) MPI_Abort(comm, MPI_ERR_ARG);
  return MPI_Irecv(buf,
		   count,
		   datatype,
		   src,
		   tag,
		   comm,
#ifdef AMPI_FORTRANCOMPATIBLE
		   request
#else 
		   &(request->plainRequest)
#endif 
		   );
}

int AMPI_Bsend(void *buf, 
	       int count, 
	       MPI_Datatype datatype, 
	       int dest, 
	       int tag,
	       AMPI_PairedWith pairedWith,
	       MPI_Comm comm) { 
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	)) MPI_Abort(comm, MPI_ERR_ARG);
  return MPI_Bsend(buf,
		   count,
		   datatype,
		   dest,
		   tag,
		   comm);
}

int AMPI_Rsend(void *buf, 
	       int count, 
	       MPI_Datatype datatype, 
	       int dest, 
	       int tag,
	       AMPI_PairedWith pairedWith,
	       MPI_Comm comm) { 
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	)) MPI_Abort(comm, MPI_ERR_ARG);
  return MPI_Rsend(buf,
		   count,
		   datatype,
		   dest,
		   tag,
		   comm);
}

int AMPI_Bcast (void* buf,
                int count,
                MPI_Datatype datatype,
                int root,
                MPI_Comm comm) {
  return MPI_Bcast(buf,
                   count,
                   datatype,
                   root,
                   comm);
}


int AMPI_Reduce (void* sbuf, 
		 void* rbuf, 
		 int count, 
		 MPI_Datatype datatype, 
		 MPI_Op op, 
		 int root, 
		 MPI_Comm comm) { 
  /* [llh] It would be nice here to call our PEDESTRIAN_AMPI_Reduce(),
   * just to test that our AMPI's reduction algorithm
   * gives the same result as the one of this MPI */
  return MPI_Reduce(sbuf,
		    rbuf,
		    count,
		    datatype,
		    op,
		    root,
		    comm);
}

int AMPI_Allreduce (void* sbuf,
                    void* rbuf,
                    int count,
                    MPI_Datatype datatype,
                    MPI_Op op,
                    MPI_Comm comm) {
  return MPI_Allreduce(sbuf,
                       rbuf,
                       count,
                       datatype,
                       op,
                       comm);
}

int AMPI_Wait(AMPI_Request *request,
	      MPI_Status *status) { 
  return MPI_Wait(
#ifdef AMPI_FORTRANCOMPATIBLE
		   request
#else 
		   &(request->plainRequest)
#endif 
		   ,status);
}

int AMPI_Waitall (int count, 
		  AMPI_Request requests[], 
		  MPI_Status statuses[]) { 
#ifndef AMPI_FORTRANCOMPATIBLE
  int i; 
  /* extract original requests */
  MPI_Request * origRequests=(MPI_Request*)malloc(count*sizeof(MPI_Request));
  assert(origRequests);
  for (i=0;i<count;++i) origRequests[i]=requests[i].plainRequest; 
#endif 
  return MPI_Waitall(count,
#ifdef AMPI_FORTRANCOMPATIBLE
		     requests,
#else
		     origRequests,
#endif
		     statuses);
}

int AMPI_Awaitall (int count, 
		   AMPI_Request requests[], 
		   MPI_Status statuses[]) { 
  return MPI_SUCCESS;
}

int AMPI_Barrier(MPI_Comm comm) {
  return MPI_Barrier(comm);
}

int AMPI_Gather(void *sendbuf,
		int sendcnt,
		MPI_Datatype sendtype,
		void *recvbuf,
		int recvcnt,
		MPI_Datatype recvtype,
		int root,
		MPI_Comm comm) { 
  return MPI_Gather(sendbuf,
		    sendcnt,
		    sendtype,
		    recvbuf,
		    recvcnt,
		    recvtype,
		    root,
		    comm);
}

int AMPI_Scatter(void *sendbuf,
		 int sendcnt,
		 MPI_Datatype sendtype,
		 void *recvbuf,
		 int recvcnt,
		 MPI_Datatype recvtype,
		 int root, 
		 MPI_Comm comm) { 
  return MPI_Scatter(sendbuf,
		     sendcnt,
		     sendtype,
		     recvbuf,
		     recvcnt,
		     recvtype,
		     root,
		     comm);
}

int AMPI_Allgather(void *sendbuf,
                   int sendcount,
                   MPI_Datatype sendtype,
                   void *recvbuf,
                   int recvcount,
                   MPI_Datatype recvtype,
                   MPI_Comm comm) {
  return MPI_Allgather(sendbuf,
                       sendcount,
                       sendtype,
                       recvbuf,
                       recvcount,
                       recvtype,
                       comm);
}

int AMPI_Gatherv(void *sendbuf,
                 int sendcnt,
                 MPI_Datatype sendtype,
                 void *recvbuf,
                 int *recvcnts,
                 int *displs,
                 MPI_Datatype recvtype,
                 int root,
                 MPI_Comm comm) {
  return MPI_Gatherv(sendbuf,
                     sendcnt,
                     sendtype,
                     recvbuf,
                     recvcnts,
                     displs,
                     recvtype,
                     root,
                     comm);
}

int AMPI_Scatterv(void *sendbuf,
                  int *sendcnts,
                  int *displs,
                  MPI_Datatype sendtype,
                  void *recvbuf,
                  int recvcnt,
                  MPI_Datatype recvtype,
                  int root, MPI_Comm comm) {
  return MPI_Scatterv(sendbuf,
                      sendcnts,
                      displs,
                      sendtype,
                      recvbuf,
                      recvcnt,
                      recvtype,
                      root,
                      comm);
}

int AMPI_Allgatherv(void *sendbuf,
                    int sendcnt,
                    MPI_Datatype sendtype,
                    void *recvbuf,
                    int *recvcnts,
                    int *displs,
                    MPI_Datatype recvtype,
                    MPI_Comm comm) {
  return MPI_Allgatherv(sendbuf,
                        sendcnt,
                        sendtype,
                        recvbuf,
                        recvcnts,
                        displs,
                        recvtype,
                        comm);
}
