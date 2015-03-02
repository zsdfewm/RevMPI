/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
/* Basic AMPI C library used for overloading and source transformation. All MPI routines
 * are subdivided into their forward _f and backward _b counterpart. The forward routines
 * are called during the forward/taping run. The backward routines are called during the
 * reverse/interpretation run.
 */

/*#define DEBUG*/
#include <ampi.h>

int * AMPI_myid;
/* Stack to compute the adjoints of the MPI reduce operation */
ampi_stack reduce_stack;

/* Non communication routines (Init, Finalize...)*/

int AMPI_Init_f(int *argc, char ***argv) {
    int ierr;
    stack_init(&reduce_stack);
    ierr=MPI_Init(argc, argv);
    return ierr;
}

int AMPI_Init_b(int *argc, char ***argv) {
    /*destroy(&reduce_stack);*/
    return MPI_Finalize();
}

int AMPI_Comm_size(MPI_Comm comm, int * numprocs) {
    return MPI_Comm_size(comm, numprocs);
}


int AMPI_Comm_rank(MPI_Comm comm, int * myid) {
    AMPI_myid = myid;
    return MPI_Comm_rank(comm, myid);
}

int AMPI_Get_processor_name(char *processor_name, int *namelen) {
    return MPI_Get_processor_name(processor_name, namelen);
}

int AMPI_Barrier(MPI_Comm comm) {
    return MPI_Barrier(comm);
}

int AMPI_Finalize_f(){
    return 1;
}

int AMPI_Finalize_b(){
    return 1;
}

/* Non blocking communication */

int AMPI_Isend_f(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request) {
#ifdef DEBUG
    int i = 0;
#endif
    request->v = buf;
    request->dest= dest;
    request->oc = AMPI_IS;
    request->size = count;
    /*request->tag = tag;*/
    request->comm = comm;
#ifdef DEBUG
    printf("AMPI_Isend_f: ");
    for(i = 0 ; i < count ; i++) {
	printf("%f ", buf[i]);
    }
    printf("\n");
#endif
    return MPI_Isend(buf, count, datatype, dest, tag, comm, &request->request);
}

int AMPI_Irecv_f(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request) {
    request->v = buf;
    request->dest= dest;
    request->oc = AMPI_IR;
    request->size = count;
    /*request->tag = tag;*/
    request->comm = comm;
    return MPI_Irecv(buf, count, datatype, dest, tag, comm, &request->request);
}

int AMPI_Wait_f(AMPI_Request *request, MPI_Status *status) {
    return MPI_Wait(&request->request, status);
}

int AMPI_Isend_b(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request) {
    int i = 0;
    if(!request->aw) {
    	MPI_Wait(&request->request, &request->status);
	for(i = 0 ; i < request->size ; i++) {
	    buf[i] = request->a[i];
	}
#ifdef DEBUG
	printf("AMPI_Isend_b: ");
	for(i = 0 ; i < request->size ; i++) {
	    /*buf[i]=1.0;*/
	    printf("%f ", buf[i]);
	}
	printf("\n");
#endif
	return MPI_SUCCESS;
    } else {
	return MPI_SUCCESS;
    }
}

int AMPI_Irecv_b(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request) {
    int i = 0;
    if(!request->aw) {
	MPI_Wait(&request->request, &request->status);
	for(i = 0 ; i < request->size ; i++) {
	    buf[i] = request->a[i];
	}
#ifdef DEBUG
	printf("AMPI_Irecv_b: ");
	for(i = 0 ; i < request->size ; i++) {
	    printf("%f ", buf[i]);
	}
	printf("\n");
#endif
	return MPI_SUCCESS;
    } else {
	return MPI_SUCCESS;
    }
}

int AMPI_Wait_b(AMPI_Request *request, MPI_Status * status) {
#ifdef DEBUG
    int i=0;
#endif
    if(request->oc == AMPI_IS) {
	return MPI_Irecv(request->a, request->size, MPI_DOUBLE, request->dest, request->tag, request->comm, &request->request);
    }
    else {
	if(request->oc == AMPI_IR) {
#ifdef DEBUG
	    printf("AMPI_Wait_recv: ");
	    for(i=0;i<request->size;i=i+1) {
		/*request->a[i]=33.0;*/
		printf("%f ",request->a[i]);
	    }
	    printf("\n");
#endif
	    return MPI_Isend(request->a, request->size, MPI_DOUBLE, request->dest, request->tag, request->comm, &request->request);
	} else { 
	    printf("Error: OC invalid\n");
	}
    }
    return MPI_SUCCESS;
}

int AMPI_Waitall_f(int count, AMPI_Request *requests, MPI_Status *status) {
    int i = 0;
    int ierr;
    MPI_Request * reqs = malloc(count * sizeof(MPI_Request*)); 

    for(i = 0 ; i < count ; i++) {
	reqs[i] = requests[i].request;
    }

    ierr = MPI_Waitall(count, reqs, status);

    for(i = 0 ; i < count ; i++) {
	requests[i].request = reqs[i];
    }
    free(reqs);
    return ierr;
}

int AMPI_Waitall_b(int count, AMPI_Request *requests) {
    MPI_Status status[count];
    int i = 0;
    for(i = 0 ; i < count ; i++) {
	AMPI_Wait_b(&requests[i], &status[i]);
    }
    return MPI_SUCCESS;
}

