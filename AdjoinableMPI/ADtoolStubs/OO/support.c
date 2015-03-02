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
#include <stdio.h>
#include "ampi/adTool/support.h"

MPI_Comm ADTOOL_AMPI_COMM_WORLD_SHADOW;

int AMPI_Init_NT(int* argc,
		 char*** argv) {
  int rc;
  rc=MPI_Init(argc,
              argv);
  ADTOOL_AMPI_setupTypes();
  ourADTOOL_AMPI_FPCollection.pushBcastInfo_fp=&ADTOOL_AMPI_pushBcastInfo;
  ourADTOOL_AMPI_FPCollection.popBcastInfo_fp=&ADTOOL_AMPI_popBcastInfo;
  ourADTOOL_AMPI_FPCollection.pushDoubleArray_fp=&ADTOOL_AMPI_pushDoubleArray;
  ourADTOOL_AMPI_FPCollection.popDoubleArray_fp=&ADTOOL_AMPI_popDoubleArray;
  ourADTOOL_AMPI_FPCollection.pushReduceInfo_fp=&ADTOOL_AMPI_pushReduceInfo; 
  ourADTOOL_AMPI_FPCollection.popReduceCountAndType_fp=&ADTOOL_AMPI_popReduceCountAndType;
  ourADTOOL_AMPI_FPCollection.popReduceInfo_fp=&ADTOOL_AMPI_popReduceInfo; 
  ourADTOOL_AMPI_FPCollection.pushSRinfo_fp=&ADTOOL_AMPI_pushSRinfo;
  ourADTOOL_AMPI_FPCollection.popSRinfo_fp=&ADTOOL_AMPI_popSRinfo;
  ourADTOOL_AMPI_FPCollection.pushOSinfo_fp=&ADTOOL_AMPI_pushOSinfo;
  ourADTOOL_AMPI_FPCollection.popOSinfo_fp=&ADTOOL_AMPI_popOSinfo;
  ourADTOOL_AMPI_FPCollection.pushGSinfo_fp=&ADTOOL_AMPI_pushGSinfo;
  ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp=&ADTOOL_AMPI_popGScommSizeForRootOrNull;
  ourADTOOL_AMPI_FPCollection.popGSinfo_fp=&ADTOOL_AMPI_popGSinfo;
  ourADTOOL_AMPI_FPCollection.pushGSVinfo_fp=&ADTOOL_AMPI_pushGSVinfo;
  ourADTOOL_AMPI_FPCollection.popGSVinfo_fp=&ADTOOL_AMPI_popGSVinfo;
  ourADTOOL_AMPI_FPCollection.push_CallCode_fp=&ADTOOL_AMPI_push_CallCode;
  ourADTOOL_AMPI_FPCollection.pop_CallCode_fp=&ADTOOL_AMPI_pop_CallCode;
  ourADTOOL_AMPI_FPCollection.push_AMPI_Request_fp=&ADTOOL_AMPI_push_AMPI_Request;
  ourADTOOL_AMPI_FPCollection.pop_AMPI_Request_fp=&ADTOOL_AMPI_pop_AMPI_Request;
  ourADTOOL_AMPI_FPCollection.push_AMPI_WinRequest_fp=&ADTOOL_AMPI_push_AMPI_WinRequest;
  ourADTOOL_AMPI_FPCollection.pop_AMPI_WinRequest_fp=&ADTOOL_AMPI_pop_AMPI_WinRequest;
  ourADTOOL_AMPI_FPCollection.push_AMPI_Win_fp=&ADTOOL_AMPI_push_AMPI_Win;
  ourADTOOL_AMPI_FPCollection.pop_AMPI_Win_fp=&ADTOOL_AMPI_pop_AMPI_Win;
  ourADTOOL_AMPI_FPCollection.push_request_fp=&ADTOOL_AMPI_push_request;
  ourADTOOL_AMPI_FPCollection.pop_request_fp=&ADTOOL_AMPI_pop_request;
  ourADTOOL_AMPI_FPCollection.push_comm_fp=&ADTOOL_AMPI_push_comm;
  ourADTOOL_AMPI_FPCollection.pop_comm_fp=&ADTOOL_AMPI_pop_comm;
  ourADTOOL_AMPI_FPCollection.rawData_fp=&ADTOOL_AMPI_rawData;
  ourADTOOL_AMPI_FPCollection.rawDataV_fp=&ADTOOL_AMPI_rawDataV;
  ourADTOOL_AMPI_FPCollection.packDType_fp=&ADTOOL_AMPI_packDType;
  ourADTOOL_AMPI_FPCollection.unpackDType_fp=&ADTOOL_AMPI_unpackDType;
  ourADTOOL_AMPI_FPCollection.writeData_fp=&ADTOOL_AMPI_writeData;
  ourADTOOL_AMPI_FPCollection.writeDataV_fp=&ADTOOL_AMPI_writeDataV;
  ourADTOOL_AMPI_FPCollection.rawAdjointData_fp=&ADTOOL_AMPI_rawAdjointData;
  ourADTOOL_AMPI_FPCollection.Turn_fp=&ADTOOL_AMPI_Turn;
  ourADTOOL_AMPI_FPCollection.mapBufForAdjoint_fp=&ADTOOL_AMPI_mapBufForAdjoint;
  ourADTOOL_AMPI_FPCollection.setBufForAdjoint_fp=&ADTOOL_AMPI_setBufForAdjoint;
  ourADTOOL_AMPI_FPCollection.getAdjointCount_fp=&ADTOOL_AMPI_getAdjointCount;
  ourADTOOL_AMPI_FPCollection.setAdjointCount_fp=&ADTOOL_AMPI_setAdjointCount;
  ourADTOOL_AMPI_FPCollection.setWinAdjointCount_fp=&ADTOOL_AMPI_setWinAdjointCount;
  ourADTOOL_AMPI_FPCollection.setWinAdjointCountAndTempBuf_fp=&ADTOOL_AMPI_setWinAdjointCountAndTempBuf;
  ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp=&ADTOOL_AMPI_allocateTempBuf;
  ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp=&ADTOOL_AMPI_releaseAdjointTempBuf;
  ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp=&ADTOOL_AMPI_allocateTempActiveBuf;
  ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp=&ADTOOL_AMPI_releaseTempActiveBuf;
  ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp=&ADTOOL_AMPI_copyActiveBuf;
  ourADTOOL_AMPI_FPCollection.adjointMultiply_fp=&ADTOOL_AMPI_adjointMultiply;
  ourADTOOL_AMPI_FPCollection.adjointMin_fp=&ADTOOL_AMPI_adjointMin;
  ourADTOOL_AMPI_FPCollection.adjointMax_fp=&ADTOOL_AMPI_adjointMax;
  ourADTOOL_AMPI_FPCollection.multiplyAdjoint_fp=&ADTOOL_AMPI_multiplyAdjoint;
  ourADTOOL_AMPI_FPCollection.divideAdjoint_fp=&ADTOOL_AMPI_divideAdjoint;
  ourADTOOL_AMPI_FPCollection.equalAdjoints_fp=&ADTOOL_AMPI_equalAdjoints;
  ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp=&ADTOOL_AMPI_incrementAdjoint;
  ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp=&ADTOOL_AMPI_nullifyAdjoint;
  ourADTOOL_AMPI_FPCollection.setupTypes_fp=&ADTOOL_AMPI_setupTypes;
  ourADTOOL_AMPI_FPCollection.cleanupTypes_fp=&ADTOOL_AMPI_cleanupTypes;
  ourADTOOL_AMPI_FPCollection.FW_rawType_fp=&ADTOOL_AMPI_FW_rawType;
  ourADTOOL_AMPI_FPCollection.BW_rawType_fp=&ADTOOL_AMPI_BW_rawType;
  ourADTOOL_AMPI_FPCollection.createWinMap_fp=&ADTOOL_AMPI_createWinMap;
  ourADTOOL_AMPI_FPCollection.writeWinData_fp=&ADTOOL_AMPI_writeWinData;
  ourADTOOL_AMPI_FPCollection.getWinSize_fp=&ADTOOL_AMPI_getWinSize;
  ourADTOOL_AMPI_FPCollection.syncAdjointWin_fp=&ADTOOL_AMPI_syncAdjointWin;
#ifdef AMPI_FORTRANCOMPATIBLE
  ourADTOOL_AMPI_FPCollection.fortransetuptypes__fp=&adtool_ampi_fortransetuptypes_;
  ourADTOOL_AMPI_FPCollection.fortrancleanuptypes__fp=&adtool_ampi_fortrancleanuptypes_;
#endif
  ourADTOOL_AMPI_FPCollection.isActiveType_fp=&ADTOOL_AMPI_isActiveType;
  ourADTOOL_AMPI_FPCollection.pushBuffer_fp=&ADTOOL_AMPI_pushBuffer ;
  ourADTOOL_AMPI_FPCollection.popBuffer_fp=&ADTOOL_AMPI_popBuffer ;
  return rc;
}

void ADTOOL_AMPI_pushBcastInfo(void* buf,
			       int count,
			       MPI_Datatype datatype,
			       int root,
			       MPI_Comm comm) {
}

void ADTOOL_AMPI_popBcastInfo(void** buf,
			      int* count,
			      MPI_Datatype* datatype,
			      int* root,
			      MPI_Comm* comm,
			      void **idx) {
}

void ADTOOL_AMPI_pushDoubleArray(void* buf,
				 int count) {
}

void ADTOOL_AMPI_popDoubleArray(double* buf,
				int* count) {
}

void ADTOOL_AMPI_pushReduceInfo(void* sbuf,
				void* rbuf,
				void* resultData,
				int pushResultData, /* push resultData if true */
				int count,
				MPI_Datatype datatype,
				MPI_Op op,
				int root,
				MPI_Comm comm) {
}
void ADTOOL_AMPI_popReduceCountAndType(int* count,
				       MPI_Datatype* datatype) {
}

void ADTOOL_AMPI_popReduceInfo(void** sbuf,
			       void** rbuf,
			       void** prevData,
			       void** resultData,
			       int* count,
			       MPI_Op* op,
			       int* root,
			       MPI_Comm* comm,
			       void **idx) {
}

void ADTOOL_AMPI_pushSRinfo(void* buf, 
			    int count,
			    MPI_Datatype datatype, 
			    int src, 
			    int tag,
			    AMPI_PairedWith pairedWith,
			    MPI_Comm comm) { 
}

void ADTOOL_AMPI_popSRinfo(void** buf,
			   int* count,
			   MPI_Datatype* datatype, 
			   int* src, 
			   int* tag,
			   AMPI_PairedWith* pairedWith,
			   MPI_Comm* comm,
			   void **idx) { 
}

void ADTOOL_AMPI_pushOSinfo(void* buf, 
			    int count,
			    MPI_Datatype datatype, 
			    int src, 
			    int tag,
			    AMPI_PairedWith pairedWith,
			    MPI_Comm comm) { 
}

void ADTOOL_AMPI_popOSinfo(void** buf,
			   int* count,
			   MPI_Datatype* datatype, 
			   int* src, 
			   int* tag,
			   AMPI_PairedWith* pairedWith,
			   MPI_Comm* comm,
			   void **idx) { 
}

void ADTOOL_AMPI_pushGSinfo(int commSizeForRootOrNull,
                            void *rbuf,
                            int rcnt,
                            MPI_Datatype rtype,
                            void *buf,
                            int count,
                            MPI_Datatype type,
                            int  root,
                            MPI_Comm comm) {
}

void ADTOOL_AMPI_popGScommSizeForRootOrNull(int *commSizeForRootOrNull) {
}

void ADTOOL_AMPI_popGSinfo(int commSizeForRootOrNull,
                           void **rbuf,
                           int *rcnt,
                           MPI_Datatype *rtype,
                           void **buf,
                           int *count,
                           MPI_Datatype *type,
                           int *root,
                           MPI_Comm *comm) {
}

void ADTOOL_AMPI_pushGSVinfo(int commSizeForRootOrNull,
                             void *rbuf,
                             int *rcnts,
                             int *displs,
                             MPI_Datatype rtype,
                             void *buf,
                             int  count,
                             MPI_Datatype type,
                             int  root,
                             MPI_Comm comm) {
}

void ADTOOL_AMPI_popGSVinfo(int commSizeForRootOrNull,
                            void **rbuf,
                            int *rcnts,
                            int *displs,
                            MPI_Datatype *rtype,
                            void **buf,
                            int *count,
                            MPI_Datatype *type,
                            int *root,
                            MPI_Comm *comm) {
}

void ADTOOL_AMPI_push_CallCode(enum AMPI_CallCode_E thisCall) { 
}

void ADTOOL_AMPI_pop_CallCode(enum AMPI_CallCode_E *thisCall) { 
}

void ADTOOL_AMPI_push_AMPI_Request(struct AMPI_Request_S  *ampiRequest) { 
}

void ADTOOL_AMPI_pop_AMPI_Request(struct AMPI_Request_S  *ampiRequest) { 
}

void ADTOOL_AMPI_push_AMPI_Win(AMPI_Win  *win) { 
}

void ADTOOL_AMPI_pop_AMPI_Win(AMPI_Win  *win) { 
}

void ADTOOL_AMPI_push_request(MPI_Request request) { 
} 

MPI_Request ADTOOL_AMPI_pop_request() { 
  return 0;
}

void ADTOOL_AMPI_push_AMPI_WinRequest(AMPI_WinRequest *winRequest) {
}

void ADTOOL_AMPI_pop_AMPI_WinRequest(AMPI_WinRequest *winRequest) {
}

void ADTOOL_AMPI_push_comm(MPI_Comm comm) {
}

MPI_Comm ADTOOL_AMPI_pop_comm() {
  return 0;
}

void* ADTOOL_AMPI_rawData(void* activeData, int *size) { 
  return activeData;
}

void* ADTOOL_AMPI_rawDataV(void* activeData, int commSize, int *counts, int* displs) {
  return activeData;
}
void * ADTOOL_AMPI_packDType(void* indata, void* outdata, int count, int idx) {
  return indata;
}
void * ADTOOL_AMPI_unpackDType(void* indata, void* outdata, int count, int idx) {
  return indata;
}

void * ADTOOL_AMPI_rawAdjointData(void* activeData) { 
  return activeData;
}

void ADTOOL_AMPI_mapBufForAdjoint(struct AMPI_Request_S  *ampiRequest,
				  void* buf) { 
}

void ADTOOL_AMPI_Turn(void* buf, void* adjointBuf) {}

void ADTOOL_AMPI_setBufForAdjoint(struct AMPI_Request_S  *ampiRequest,
				  void* buf) { 
  /* do nothing */
}

void ADTOOL_AMPI_getAdjointCount(int *count,
				 MPI_Datatype datatype) { 
}

void ADTOOL_AMPI_setAdjointCount(struct AMPI_Request_S  *ampiRequest) { 
  /* for now we keep the count as is but for example in vector mode one would have to multiply by vector length */
}
void ADTOOL_AMPI_setAdjointCountAndTempBuf(struct AMPI_Request_S *ampiRequest) { 
  ADTOOL_AMPI_setAdjointCount(ampiRequest);
  ampiRequest->adjointTempBuf =
    ADTOOL_AMPI_allocateTempBuf(ampiRequest->adjointCount,
                                ampiRequest->datatype,
                                ampiRequest->comm) ;
  assert(ampiRequest->adjointTempBuf);
}

void ADTOOL_AMPI_setWinAdjointCount(AMPI_WinRequest *winRequest) { 
}

void ADTOOL_AMPI_setWinAdjointCountAndTempBuf(AMPI_WinRequest *winRequest) { 
  ADTOOL_AMPI_setWinAdjointCount(winRequest);
  assert(winRequest->adjointTempBuf);
}

void ADTOOL_AMPI_syncAdjointWin(AMPI_Win *win) { 
}

void* ADTOOL_AMPI_allocateTempBuf(int adjointCount, MPI_Datatype datatype, MPI_Comm comm) {
  size_t s=0;
  int dt_idx = derivedTypeIdx(datatype);
  if (datatype==MPI_DOUBLE)
    s=sizeof(double);
  else if (datatype==MPI_FLOAT)
    s=sizeof(float);
  else if (isDerivedType(dt_idx))
    s = getDTypeData()->p_extents[dt_idx];
  else
    MPI_Abort(comm, MPI_ERR_TYPE);
  return (void*)malloc(adjointCount*s);
}

void ADTOOL_AMPI_releaseAdjointTempBuf(void *tempBuf) { 
  free(tempBuf) ;
}

void* ADTOOL_AMPI_allocateTempActiveBuf(int count,
					MPI_Datatype datatype,
					MPI_Comm comm) {
  void* ptr = NULL;
  if (datatype==MPI_DOUBLE)
    ptr = malloc(count*sizeof(MPI_DOUBLE));
  else if (datatype==MPI_FLOAT)
    ptr = malloc(count*sizeof(MPI_FLOAT));
  assert(ptr);
  return ptr;
}

void ADTOOL_AMPI_releaseTempActiveBuf(void *buf,
				      int count,
				      MPI_Datatype datatype) {
  free(buf);
}

void * ADTOOL_AMPI_copyActiveBuf(void* source,
                                 void* target,
                                 int count,
                                 MPI_Datatype datatype,
                                 MPI_Comm comm) {
  return source;
}

/** This is the adjoint of assignment target=source*target */
void ADTOOL_AMPI_adjointMultiply(int count, MPI_Datatype datatype, MPI_Comm comm,
                                 void *source, void *adjointSource,
                                 void* target, void* adjointTarget) {
}

/** This is the adjoint of assignment target=MIN(source,target) */
void ADTOOL_AMPI_adjointMin(int count, MPI_Datatype datatype, MPI_Comm comm,
                                 void *source, void *adjointSource,
                                 void* target, void* adjointTarget) {
}

/** This is the adjoint of assignment target=MAX(source,target) */
void ADTOOL_AMPI_adjointMax(int count, MPI_Datatype datatype, MPI_Comm comm,
                                 void *source, void *adjointSource,
                                 void* target, void* adjointTarget) {
}

void ADTOOL_AMPI_incrementAdjoint(int adjointCount, MPI_Datatype datatype, MPI_Comm comm, void* target, void *source, void *idx) { 
}

void ADTOOL_AMPI_multiplyAdjoint(int adjointCount, MPI_Datatype datatype, MPI_Comm comm, void* target, void *source, void *idx) {
}

void ADTOOL_AMPI_divideAdjoint(int adjointCount, MPI_Datatype datatype, MPI_Comm comm, void* target, void *source, void *idx) {
}

void ADTOOL_AMPI_equalAdjoints(int adjointCount, MPI_Datatype datatype, MPI_Comm comm, void* target, void *source1, void *source2, void *idx) {
}

void ADTOOL_AMPI_nullifyAdjoint(int adjointCount, MPI_Datatype datatype, MPI_Comm comm, void* target) { 
}

/**
 * Push the contents of buffer somewhere
 */
void ADTOOL_AMPI_pushBuffer(int count, MPI_Datatype datatype, MPI_Comm comm,
                            void* buffer) {
  printf("Please provide implementation of ADTOOL_AMPI_pushBuffer()\n") ;
}

/**
 * Pop the contents of buffer from somewhere
 */
void ADTOOL_AMPI_popBuffer(int count, MPI_Datatype datatype, MPI_Comm comm,
                           void* buffer) {
  printf("Please provide implementation of ADTOOL_AMPI_popBuffer()\n") ;
}

void ADTOOL_AMPI_writeData(void *buf,int *count) { }

void ADTOOL_AMPI_writeDataV(void* activeData, int *counts, int* displs) {}

AMPI_Activity ADTOOL_AMPI_isActiveType(MPI_Datatype datatype) {
  return AMPI_PASSIVE;
}

void *ADTOOL_AMPI_createWinMap(void *active_buf, MPI_Aint size){
  return NULL;
}

void ADTOOL_AMPI_writeWinData(void *map, void *buf, MPI_Aint size){
}

MPI_Aint ADTOOL_AMPI_getWinSize(MPI_Aint size) {
   return 0;
}

#ifdef AMPI_FORTRANCOMPATIBLE
  void adtool_ampi_fortransetuptypes_(MPI_Fint* adouble,MPI_Fint* areal) {
  }
#endif

void ADTOOL_AMPI_setupTypes() {
#ifdef AMPI_FORTRANCOMPATIBLE
  MPI_Fint adouble;
  MPI_Fint areal;
#endif
  /* Change AMPI_ADOUBLE to something else? Need AMPI_ADOUBLE!=MPI_DOUBLE for derived types. */
  AMPI_ADOUBLE=MPI_DOUBLE;
  AMPI_AFLOAT=MPI_FLOAT;
#ifdef AMPI_FORTRANCOMPATIBLE
  adtool_ampi_fortransetuptypes_(&adouble, &areal);
  AMPI_ADOUBLE_PRECISION=MPI_Type_f2c(adouble);
  AMPI_AREAL=MPI_Type_f2c(areal);
#endif
}

#ifdef AMPI_FORTRANCOMPATIBLE
  void adtool_ampi_fortrancleanuptypes_(MPI_Fint* adouble,MPI_Fint* areal) {
  }
#endif

void ADTOOL_AMPI_cleanupTypes() {
}

MPI_Datatype ADTOOL_AMPI_FW_rawType(MPI_Datatype datatype) {
  return datatype;
}

MPI_Datatype ADTOOL_AMPI_BW_rawType(MPI_Datatype datatype) {
  return datatype;
}
