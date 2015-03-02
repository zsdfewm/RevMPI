/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#ifndef AMPI_TAPE_INCLUDE
#define AMPI_TAPE_INCLUDE

/* Generic AMPI C tape. Don't touch this. The code is always mirrored with the AMPI repo.
 * Changing code here will result in merge conflicts. 
 */

#define AMPI_CHUNK_SIZE 500000
#define ASSERT 
#define ISEND 3
#define IRECV 4
#define WAIT 5
#define WAITALL 6
#define AWAITALL 7

#include <stdlib.h>
#include <assert.h>
#include <ampi.h>

/*int ampi_vac=0;*/

typedef struct ampi_tape_entry {
    int oc;
    int *arg;
    INT64 idx;
    AMPI_Request *request;
    MPI_Comm comm;
    int tag;
}ampi_tape_entry;

/* AMPI taping routines which have an MPI counterpart that is adjoined.*/

int AMPI_Reset_Tape();
int AMPI_Init(int*, char***);
int AMPI_Finalize();

int AMPI_Send(void*, int, MPI_Datatype, int, int, MPI_Comm);
int AMPI_Recv(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Status*);

int AMPI_Isend(void*, int, MPI_Datatype, int, int, MPI_Comm, AMPI_Request *);
int AMPI_Irecv(void*, int, MPI_Datatype, int, int, MPI_Comm, AMPI_Request *);

int AMPI_Wait(AMPI_Request *, MPI_Status *);
int AMPI_Waitall(int , AMPI_Request *, MPI_Status *);
int AMPI_Awaitall(int , AMPI_Request *, MPI_Status *);

/* AMPI routines that have to be called by the external tape */

/* Call AMPI tape interpreter from AD tool tape */
void ampi_interpret_tape(void);

/* Call AMPI tape printer from external tape printer (optional) */
void ampi_print_tape(void);
/* Call AMPI tape printer from external tape printer for one tape entry (optional)*/
void ampi_print_tape_entry(int *j);
void ampi_check_tape_size(int size);

/* AMPI routines which are defined as external. These routines need to be implemented by
 * the AD tool tape library. They should implement the data flow between the external
 * tape and the AMPI tape.
 */

/* Get a value v from a specific tape entry with index idx */
extern void ampi_get_val(void* buf, int* i, double* v);

/* Get a value v from a specific tape variable buf[i] */
extern void ampi_set_val(void* buf, int* i, double* v);

/* Get an adjoint a from a specific tape entry with index idx */
extern void ampi_get_adj(INT64* idx, double* a);

/* Set an adjoint a from a specific tape entry with index idx */
extern void ampi_set_adj(INT64*, double*);

/* Get a tape index from a variable buf[i] */
extern void ampi_get_idx(void* buf, int* i, INT64* idx);

/* Create a tape entry in the external tape, indicating an external AMPI call */
extern void ampi_create_tape_entry(int* i);

/* Create size tape entries to store the values of buf. Refer to receive buffer without
 * initialization */
extern void ampi_create_dummies(void* buf, int *size);
#endif
