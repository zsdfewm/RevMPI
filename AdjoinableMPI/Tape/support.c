/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "ampi/tape/support.h"

static void* myTapeStorage=0;
static size_t myTapeStorageSize=0;
static void* myStack_p=0;
static void* myRead_p=0;
static void* myStackTop_p=0;

void TAPE_AMPI_init() {
  /* reset things */
  if (myTapeStorage) {
    free(myTapeStorage);
  }
  myTapeStorage=0;
  myTapeStorageSize=0;
  myStack_p=myTapeStorage;
  myRead_p=myTapeStorage;
  myStackTop_p=myTapeStorage;
}


static void writeBlob(void*, size_t);
static void readBlob(void*,size_t);

void writeBlob(void * aBlob,size_t aSize) {
  assert(aBlob);
  /* make some space*/
  if (aSize>(char*)myTapeStorage+myTapeStorageSize-(char*)myStack_p) {
    size_t increment=0;
    void *newTapeStorage=0;
    if (increment<aSize) increment=aSize;
    if (increment<1024) increment=1024;
    myTapeStorageSize=myTapeStorageSize+increment;
    newTapeStorage=realloc(myTapeStorage,myTapeStorageSize);
    assert(newTapeStorage);
    if (newTapeStorage!=myTapeStorage) {
      myStack_p=(char*)newTapeStorage+((char*)myStack_p-(char*)myTapeStorage);
      myRead_p=(char*)newTapeStorage+((char*)myRead_p-(char*)myTapeStorage);
      myTapeStorage=newTapeStorage;
    }
  }
  memcpy(myStack_p,aBlob,aSize);
  myStack_p=(char*)myStack_p+aSize;
  myStackTop_p=myStack_p;
}

void readBlob(void* aBlob,size_t aSize) {
  assert(aSize<=(char*)myTapeStorage+myTapeStorageSize-(char*)myRead_p);
  memcpy(aBlob,myRead_p,aSize);
  myRead_p=(char*)myRead_p+aSize;
}

void popBlob(void* aBlob,size_t aSize) {
  assert(aSize<=(char*)myStack_p-(char*)myTapeStorage);
  myStack_p=(char*)myStack_p-aSize;
  memcpy(aBlob,myStack_p,aSize);
}

void TAPE_AMPI_resetBottom() {
  myRead_p=myTapeStorage;
}

void TAPE_AMPI_resetTop() {
  myStack_p=myStackTop_p;
}

void TAPE_AMPI_push_int(int an_int)  { writeBlob((void*)(&an_int),sizeof(int)); }
void TAPE_AMPI_pop_int(int *an_int)  { popBlob((void*)(an_int),sizeof(int)); }
void TAPE_AMPI_read_int(int* an_int) { readBlob((void*)(an_int),sizeof(int)); }

void TAPE_AMPI_push_MPI_Aint(MPI_Aint an_MPI_Aint)  { writeBlob((void*)(&an_MPI_Aint),sizeof(MPI_Aint)); }
void TAPE_AMPI_pop_MPI_Aint(MPI_Aint *an_MPI_Aint)  { popBlob((void*)(an_MPI_Aint),sizeof(MPI_Aint)); }
void TAPE_AMPI_read_MPI_Aint(MPI_Aint* an_MPI_Aint) { readBlob((void*)(an_MPI_Aint),sizeof(MPI_Aint)); }

void TAPE_AMPI_push_ptr(void *an_int)  { writeBlob((void*)(&an_int),sizeof(void*)); }
void TAPE_AMPI_pop_ptr(void **ptr)  { popBlob((void*)(ptr),sizeof(void*)); }
void TAPE_AMPI_read_ptr(void **ptr) { readBlob((void*)(ptr),sizeof(void*)); }

void TAPE_AMPI_push_MPI_Datatype(MPI_Datatype an_MPI_Datatype)  { writeBlob((void*)(&an_MPI_Datatype),sizeof(MPI_Datatype)); }
void TAPE_AMPI_pop_MPI_Datatype(MPI_Datatype *an_MPI_Datatype)  { popBlob((void*)(an_MPI_Datatype),sizeof(MPI_Datatype)); }
void TAPE_AMPI_read_MPI_Datatype(MPI_Datatype* an_MPI_Datatype) { readBlob((void*)(an_MPI_Datatype),sizeof(MPI_Datatype)); }

void TAPE_AMPI_push_MPI_Comm(MPI_Comm an_MPI_Comm)  { writeBlob((void*)(&an_MPI_Comm),sizeof(MPI_Comm)); }
void TAPE_AMPI_pop_MPI_Comm(MPI_Comm *an_MPI_Comm)  { popBlob((void*)(an_MPI_Comm),sizeof(MPI_Comm)); }
void TAPE_AMPI_read_MPI_Comm(MPI_Comm* an_MPI_Comm) { readBlob((void*)(an_MPI_Comm),sizeof(MPI_Comm)); }

void TAPE_AMPI_push_MPI_Request(MPI_Request an_MPI_Request)  { writeBlob((void*)(&an_MPI_Request),sizeof(MPI_Request)); }
void TAPE_AMPI_pop_MPI_Request(MPI_Request *an_MPI_Request)  { popBlob((void*)(an_MPI_Request),sizeof(MPI_Request)); }
void TAPE_AMPI_read_MPI_Request(MPI_Request* an_MPI_Request) { readBlob((void*)(an_MPI_Request),sizeof(MPI_Request)); }

void TAPE_AMPI_push_MPI_Op(MPI_Op an_MPI_Op)  { writeBlob((void*)(&an_MPI_Op),sizeof(MPI_Op)); }
void TAPE_AMPI_pop_MPI_Op(MPI_Op *an_MPI_Op)  { popBlob((void*)(an_MPI_Op),sizeof(MPI_Op)); }
void TAPE_AMPI_read_MPI_Op(MPI_Op* an_MPI_Op) { readBlob((void*)(an_MPI_Op),sizeof(MPI_Op)); }

void TAPE_AMPI_push_double(double a_double)  { writeBlob((void*)(&a_double),sizeof(double)); }
void TAPE_AMPI_pop_double(double *a_double)  { popBlob((void*)(a_double),sizeof(double)); }
void TAPE_AMPI_read_double(double* a_double) { readBlob((void*)(a_double),sizeof(double)); }

void TAPE_AMPI_push_MPI_Win(MPI_Win an_MPI_Win)  { writeBlob((void*)(&an_MPI_Win),sizeof(MPI_Win)); }
void TAPE_AMPI_pop_MPI_Win(MPI_Win *an_MPI_Win)  { popBlob((void*)(an_MPI_Win),sizeof(MPI_Win)); }
void TAPE_AMPI_read_MPI_Win(MPI_Win * an_MPI_Win) { readBlob((void*)(an_MPI_Win),sizeof(MPI_Win)); }

