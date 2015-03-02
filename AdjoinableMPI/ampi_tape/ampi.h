/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#ifndef AMPI_H
#define AMPI_H
/* Basic AMPI C library used for overloading and source transformation. All MPI routines
 * are subdivided into their forward _f and backward _b counterpart. The forward routines
 * are called during the forward/taping run. The backward routines are called during the
 * reverse/interpretation run.
 */
#include <mpi.h>
#include <ampi_stack.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#define AMPI_IS 1
#define AMPI_IR 2

#define AMPI_COMM_WORLD MPI_COMM_WORLD
#define AMPI_MAX_PROCESSOR_NAME MPI_MAX_PROCESSOR_NAME
#define AMPI_Status MPI_Status
#define AMPI_DOUBLE MPI_DOUBLE
/*#define INT64 double**/
#define INT64 int

/* Reduce operations */

#define AMPI_SUM   1
#define AMPI_PROD  2
#define AMPI_MIN   3
#define AMPI_MAX   4


/* AMPI request, replacing the MPI request */
typedef struct AMPI_Request {
    MPI_Request request;
    MPI_Status status;
    MPI_Comm comm;

    /*special for tape*/
    void *buf;

    int tag;
    double *v;
    double *a;
    int va;
    int oc;
    int dest;
    int size;
    int aw;
} AMPI_Request;

/* Non communication routines (Init, Finalize...)*/

int AMPI_Init_f(int*, char***);
int AMPI_Init_b(int*, char***);
int AMPI_Comm_size(MPI_Comm, int*);
int AMPI_Comm_rank(MPI_Comm, int*);
int AMPI_Get_processor_name(char*, int*);
int AMPI_Barrier(MPI_Comm);
int AMPI_Finalize_f();
int AMPI_Finalize_b();

/* Non blocking communication */

int AMPI_Isend_f(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request);
int AMPI_Irecv_f(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request);
int AMPI_Wait_f(AMPI_Request *request , MPI_Status *status);
int AMPI_Isend_b(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request);
int AMPI_Irecv_b(double *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request);
int AMPI_Wait_b(AMPI_Request *request, MPI_Status *status);
int AMPI_Waitall_f(int count, AMPI_Request *requests, MPI_Status *status);
int AMPI_Waitall_b(int count, AMPI_Request *requests);

#endif
