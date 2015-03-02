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
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include "ampi/libCommon/modified.h"
#include "ampi/bookkeeping/support.h"
#include "ampi/adTool/support.h"

MPI_Datatype AMPI_ADOUBLE;
MPI_Datatype AMPI_AFLOAT;

#ifdef AMPI_FORTRANCOMPATIBLE
MPI_Datatype AMPI_ADOUBLE_PRECISION;
MPI_Datatype AMPI_AREAL;
#endif

int FW_AMPI_Recv(void* buf, 
		 int count,
		 MPI_Datatype datatype, 
		 int src, 
		 int tag,
		 AMPI_PairedWith pairedWith,
		 MPI_Comm comm,
		 MPI_Status* status) { 
  int rc=0;
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
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else { 
    MPI_Status myStatus;
    double* mappedbuf=NULL;
    int dt_idx = derivedTypeIdx(datatype);
    int is_derived = isDerivedType(dt_idx);
    if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      mappedbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(buf,&count);
    }
    else if(is_derived) {
      mappedbuf=(*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,datatype,comm);
    }
    else {
      mappedbuf=buf;
    }
    rc=MPI_Recv(mappedbuf,
		count,
		(*ourADTOOL_AMPI_FPCollection.FW_rawType_fp)(datatype),
		src,
		tag,
		comm,
		&myStatus); /* because status as passed in may be MPI_STATUS_IGNORE */
    if (rc==MPI_SUCCESS && ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE || is_derived)) {
      if (is_derived) {
	(*ourADTOOL_AMPI_FPCollection.unpackDType_fp)(mappedbuf,buf,count,dt_idx);
	(*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(mappedbuf);
      }
      (*ourADTOOL_AMPI_FPCollection.writeData_fp)(buf,&count);
      if(tag==MPI_ANY_TAG) tag=myStatus.MPI_TAG;
      if(src==MPI_ANY_SOURCE) src=myStatus.MPI_SOURCE;
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_RECV);
      (*ourADTOOL_AMPI_FPCollection.pushSRinfo_fp)(buf,
						   count,
						   datatype,
						   src,
						   tag,
						   pairedWith,
						   comm);
    }
    if (status!=MPI_STATUS_IGNORE) *status=myStatus;
  }
  return rc;
}  

int BW_AMPI_Recv(void* buf, 
		 int count,
		 MPI_Datatype datatype, 
		 int src, 
		 int tag,
		 AMPI_PairedWith pairedWith,
		 MPI_Comm comm,
		 MPI_Status* status) {
  int rc=0;
  void *idx=NULL;
  (*ourADTOOL_AMPI_FPCollection.popSRinfo_fp)(&buf,
					      &count,
					      &datatype,
					      &src,
					      &tag,
					      &pairedWith,
					      &comm,
					      &idx);
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
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else { 
    switch(pairedWith) { 
    case AMPI_FROM_ISEND_WAIT:
    case AMPI_FROM_SEND: {
      MPI_Datatype mappedtype = (*ourADTOOL_AMPI_FPCollection.BW_rawType_fp)(datatype);
      (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&count,datatype);   
      rc=MPI_Send(buf,
		  count,
		  mappedtype,
		  src,
		  tag,
		  comm);
      (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,mappedtype,comm, buf);
      break;
    }
    default:  
      rc=MPI_Abort(comm, MPI_ERR_TYPE);
      break;
    }
  }
  return rc;
}  

int TLM_AMPI_Recv(void* buf,
                  int count,
                  MPI_Datatype datatype,
                  int src,
                  int tag,
                  AMPI_PairedWith pairedWith,
                  MPI_Comm comm,
                  MPI_Status* status) {
  int rc;
  rc = MPI_Recv(buf,count,datatype,src,tag,comm,status);
  return rc;
}

/**
 * Tangent Recv, with separate shadow (i.e. tangent) buffer.
 */
int TLS_AMPI_Recv(void* buf, void* shadowbuf,
                  int count,
                  MPI_Datatype datatype, MPI_Datatype shadowdatatype,
                  int src,
                  int tag,
                  AMPI_PairedWith pairedWith,
                  MPI_Comm comm,
                  MPI_Status* status) {
  MPI_Status status1 ;
  int rc = MPI_Recv(buf, count, datatype, src, tag, comm, &status1) ;
  assert(rc==MPI_SUCCESS);
  MPI_Comm shadowcomm = (*ourADTOOL_AMPI_FPCollection.getShadowComm_fp)(comm) ;
  rc = MPI_Recv(shadowbuf, count, shadowdatatype,
                    (src==MPI_ANY_SOURCE?status1.MPI_SOURCE:src),
                    (tag==MPI_ANY_TAG?status1.MPI_TAG:tag),
                    shadowcomm, status) ;
  assert(rc==MPI_SUCCESS);
  return rc ;
}
                  

int FW_AMPI_Irecv (void* buf,
		   int count,
		   MPI_Datatype datatype,
		   int source,
		   int tag,
		   AMPI_PairedWith pairedWith,
		   MPI_Comm comm,
		   AMPI_Request* request) {
  int rc=0;
  if (!(
	pairedWith==AMPI_FROM_SEND
        ||
        pairedWith==AMPI_FROM_ISEND_WAIT
	||
	pairedWith==AMPI_FROM_ISEND_WAITALL
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else {
    double* mappedbuf=NULL;
    if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      mappedbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(buf,&count);
    }
    else {
      mappedbuf=buf;
    }
    rc= MPI_Irecv(mappedbuf,
		  count,
		  datatype,
		  source,
		  tag,
		  comm,
#ifdef AMPI_FORTRANCOMPATIBLE
                  request
#else
                  &(request->plainRequest)
#endif
		  );
    struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
    struct AMPI_Request_S ampiRequestInst;
    ampiRequest=&ampiRequestInst;
    ampiRequest->plainRequest=*request;
#else 
    ampiRequest=request;
#endif
    /* fill in the other info */
    ampiRequest->endPoint=source;
    ampiRequest->tag=tag;
    ampiRequest->count=count;
    ampiRequest->datatype=datatype;
    ampiRequest->comm=comm;
    ampiRequest->origin=AMPI_RECV_ORIGIN;
    ampiRequest->pairedWith=pairedWith;
    (*ourADTOOL_AMPI_FPCollection.mapBufForAdjoint_fp)(ampiRequest,buf);
    ampiRequest->tracedRequest=ampiRequest->plainRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
    BK_AMPI_put_AMPI_Request(ampiRequest);
#endif
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_IRECV);
#ifdef AMPI_REQUESTONTRACE
      (*ourADTOOL_AMPI_FPCollection.push_request_fp)(ampiRequest->tracedRequest);
#endif
    }
  }
  return rc;
}

int BW_AMPI_Irecv (void* buf, 
		   int count, 
		   MPI_Datatype datatype, 
		   int source, 
		   int tag,
		   AMPI_PairedWith pairedWith,
		   MPI_Comm comm, 
		   AMPI_Request* request) {
  int rc=0;
  MPI_Request *plainRequest;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_REQUESTONTRACE
  MPI_Request tracedRequest;
#endif
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  plainRequest=request;
#else
  plainRequest=&(request->plainRequest) ;
  ampiRequest=request;
#endif
#if defined AMPI_FORTRANCOMPATIBLE || defined AMPI_REQUESTONTRACE
#ifdef AMPI_REQUESTONTRACE
  tracedRequest=(*ourADTOOL_AMPI_FPCollection.pop_request_fp)();
  BK_AMPI_get_AMPI_Request(&tracedRequest,ampiRequest,1);
#else 
  BK_AMPI_get_AMPI_Request(plainRequest,ampiRequest,0);
#endif
#endif
  assert(ampiRequest->origin==AMPI_RECV_ORIGIN) ;
  if (!(
	ampiRequest->pairedWith==AMPI_FROM_SEND 
	|| 
	ampiRequest->pairedWith==AMPI_FROM_ISEND_WAIT
	||
	ampiRequest->pairedWith==AMPI_FROM_ISEND_WAITALL
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else { 
    switch(ampiRequest->pairedWith) { 
    case AMPI_FROM_SEND:
    case AMPI_FROM_ISEND_WAIT: {
      rc=MPI_Wait(plainRequest,
		  MPI_STATUS_IGNORE);
      (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(ampiRequest->adjointCount,
                                                       ampiRequest->datatype,
                                                       ampiRequest->comm,
                                                       ampiRequest->adjointBuf);
      break ;
    }
    default:  
      rc=MPI_Abort(ampiRequest->comm, MPI_ERR_TYPE);
      break;
    }
  }
  return rc;
}

int TLM_AMPI_Irecv (void* buf,
                    int count,
                    MPI_Datatype datatype,
                    int source,
                    int tag,
                    AMPI_PairedWith pairedWith,
                    MPI_Comm comm,
                    AMPI_Request* request) {
  int rc=0;
  assert(0);
  return rc;
}

/**
 * Tangent Irecv, with separate shadow (i.e. tangent) buffer.
 */
int TLS_AMPI_Irecv (void* buf, void* shadowbuf,
                    int count,
                    MPI_Datatype datatype, MPI_Datatype shadowdatatype,
                    int source,
                    int tag,
                    AMPI_PairedWith pairedWith,
                    MPI_Comm comm,
                    AMPI_Request* request) {

  int rc=0;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  ampiRequest->plainRequest=*request;
#else 
  ampiRequest=request;
#endif
  /* fill in the info needed to Recv the shadowbuf later.
  * [llh]: I don't need pairedWith nor tracedRequest... */
  ampiRequest->endPoint=source;
  ampiRequest->tag=tag;
  ampiRequest->count=count;
  ampiRequest->datatype=shadowdatatype;
  ampiRequest->comm=comm;
  ampiRequest->origin=AMPI_RECV_ORIGIN;
  ampiRequest->pairedWith=pairedWith;
  ampiRequest->adjointBuf=shadowbuf ;
  ampiRequest->tracedRequest=ampiRequest->plainRequest;
  rc= MPI_Irecv(buf,
                count,
                datatype,
                source,
                tag,
                comm,
                &(ampiRequest->plainRequest));
#ifdef AMPI_FORTRANCOMPATIBLE
  *request = ampiRequest->plainRequest ;
  BK_AMPI_put_AMPI_Request(ampiRequest);
#endif
  return rc;
}

int FW_AMPI_Send (void* buf, 
                  int count, 
                  MPI_Datatype datatype, 
                  int dest, 
                  int tag,
                  AMPI_PairedWith pairedWith,
                  MPI_Comm comm) {
  int rc=0;
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	||
	pairedWith==AMPI_TO_IRECV_WAITALL
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else {
    double* mappedbuf=NULL;
    int dt_idx = derivedTypeIdx(datatype);
    int is_derived = isDerivedType(dt_idx);
    if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      mappedbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(buf,&count);
    }
    else if(is_derived) {
      mappedbuf=(*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,datatype,comm);
      (*ourADTOOL_AMPI_FPCollection.packDType_fp)(buf,mappedbuf,count,dt_idx);
    }
    else {
      mappedbuf=buf;
    }
    rc=MPI_Send(mappedbuf,
		count,
		(*ourADTOOL_AMPI_FPCollection.FW_rawType_fp)(datatype),
		/* if derived then need to replace typemap */
		dest,
		tag,
		comm);
    if (is_derived) (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(mappedbuf);
    if (rc==MPI_SUCCESS && ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE || is_derived)) {
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_SEND);
      (*ourADTOOL_AMPI_FPCollection.pushSRinfo_fp)(buf,
						   count,
						   datatype,
						   dest,
						   tag,
						   pairedWith,
						   comm);
    }
  }
  return rc;
}

int BW_AMPI_Send (void* buf,
                  int count, 
                  MPI_Datatype datatype, 
                  int dest, 
                  int tag,
                  AMPI_PairedWith pairedWith,
                  MPI_Comm comm) {
  int rc=0;
  void *idx=NULL;
  (*ourADTOOL_AMPI_FPCollection.popSRinfo_fp)(&buf,
					      &count,
					      &datatype,
					      &dest,
					      &tag,
					      &pairedWith,
					      &comm,
					      &idx);
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	||
	pairedWith==AMPI_TO_IRECV_WAITALL
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else {
    switch(pairedWith) {
    case AMPI_TO_IRECV_WAIT:
    case AMPI_TO_RECV: { 
      MPI_Datatype mappedtype = (*ourADTOOL_AMPI_FPCollection.BW_rawType_fp)(datatype);
      (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&count,datatype);
      void *tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm) ;
      rc=MPI_Recv(tempBuf,
                  count,
                  mappedtype,
                  dest,
                  tag,
                  comm,
                  MPI_STATUS_IGNORE) ;
      (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count,
                                                         mappedtype,
                                                         comm,
                                                         buf,
                                                         tempBuf, idx);
      (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
      break;
    }
    default:  
      rc=MPI_Abort(comm, MPI_ERR_TYPE);
      break;
    }
  }
  return rc;
}

int TLM_AMPI_Send (void* buf,
                   int count,
                   MPI_Datatype datatype,
                   int dest,
                   int tag,
                   AMPI_PairedWith pairedWith,
                   MPI_Comm comm) {
  int rc=0;
  MPI_Send(buf,count,datatype,dest,tag,comm);
  return rc;
}

/**
 * Tangent Send, with separate shadow (i.e. tangent) buffer.
 */
int TLS_AMPI_Send (void* buf,  void* shadowbuf,
                   int count,
                   MPI_Datatype datatype, MPI_Datatype shadowdatatype,
                   int dest,
                   int tag,
                   AMPI_PairedWith pairedWith,
                   MPI_Comm comm) {
  int rc = MPI_Send(buf, count, datatype, dest, tag, comm) ;
  assert(rc==MPI_SUCCESS);
  MPI_Comm shadowcomm = (*ourADTOOL_AMPI_FPCollection.getShadowComm_fp)(comm) ;
  rc = MPI_Send(shadowbuf, count, shadowdatatype, dest, tag, shadowcomm) ;
  assert(rc==MPI_SUCCESS);
  return rc ;
}

int FW_AMPI_Isend (void* buf,
		   int count, 
		   MPI_Datatype datatype, 
		   int dest, 
		   int tag,
		   AMPI_PairedWith pairedWith,
		   MPI_Comm comm, 
		   AMPI_Request* request) { 
  int rc=0;
  if (!(
	pairedWith==AMPI_TO_RECV 
	|| 
	pairedWith==AMPI_TO_IRECV_WAIT
	||
	pairedWith==AMPI_TO_IRECV_WAITALL
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else { 
    double* mappedbuf=NULL;
    if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      mappedbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(buf,&count);
    }
    else {
      mappedbuf=buf;
    }
    rc= MPI_Isend(mappedbuf,
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
    struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
    struct AMPI_Request_S ampiRequestInst;
    ampiRequest=&ampiRequestInst;
    ampiRequest->plainRequest=*request;
#else 
    ampiRequest=request;
#endif
    /* fill in the other info */
    ampiRequest->endPoint=dest;
    ampiRequest->tag=tag;
    ampiRequest->count=count;
    ampiRequest->datatype=datatype;
    ampiRequest->comm=comm;
    ampiRequest->origin=AMPI_SEND_ORIGIN;
    ampiRequest->pairedWith=pairedWith;
    (*ourADTOOL_AMPI_FPCollection.mapBufForAdjoint_fp)(ampiRequest,buf);
    ampiRequest->tracedRequest=ampiRequest->plainRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
    BK_AMPI_put_AMPI_Request(ampiRequest);
#endif
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_ISEND);
#ifdef AMPI_REQUESTONTRACE
      (*ourADTOOL_AMPI_FPCollection.push_request_fp)(ampiRequest->tracedRequest);
#endif
    }
  }
  return rc;
}

int BW_AMPI_Isend (void* buf, 
		   int count, 
		   MPI_Datatype datatype, 
		   int dest, 
		   int tag,
		   AMPI_PairedWith pairedWith,
		   MPI_Comm comm, 
		   AMPI_Request* request) { 
  int rc=0;
  MPI_Request *plainRequest;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_REQUESTONTRACE
  MPI_Request tracedRequest;
#endif
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  plainRequest=request;
#else 
  ampiRequest=request;
  plainRequest=&(ampiRequest->plainRequest);
#endif
#if defined AMPI_FORTRANCOMPATIBLE || defined AMPI_REQUESTONTRACE
#ifdef AMPI_REQUESTONTRACE
  tracedRequest=(*ourADTOOL_AMPI_FPCollection.pop_request_fp)();
  BK_AMPI_get_AMPI_Request(&tracedRequest,ampiRequest,1);
#else 
  BK_AMPI_get_AMPI_Request(plainRequest,ampiRequest,0);
#endif
#endif
  assert(ampiRequest->origin==AMPI_SEND_ORIGIN) ;
  if (!(
	ampiRequest->pairedWith==AMPI_TO_RECV 
	|| 
	ampiRequest->pairedWith==AMPI_TO_IRECV_WAIT
	||
	ampiRequest->pairedWith==AMPI_TO_IRECV_WAITALL
	)) rc=MPI_Abort(comm, MPI_ERR_ARG);
  else { 
    switch(ampiRequest->pairedWith) { 
    case AMPI_TO_RECV:
    case AMPI_TO_IRECV_WAIT: { 
      rc=MPI_Wait(plainRequest,
		  MPI_STATUS_IGNORE);
      (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(ampiRequest->adjointCount,
                                                         ampiRequest->datatype,
                                                         ampiRequest->comm,
                                                         ampiRequest->adjointBuf,
                                                         ampiRequest->adjointTempBuf,
                                                         ampiRequest->idx);
      (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(ampiRequest->adjointTempBuf);
      break;
    }
    default:  
      rc=MPI_Abort(ampiRequest->comm, MPI_ERR_TYPE);
      break;
    }
  }
  return rc;
}

int TLM_AMPI_Isend (void* buf,
                    int count,
                    MPI_Datatype datatype,
                    int dest,
                    int tag,
                    AMPI_PairedWith pairedWith,
                    MPI_Comm comm,
                    AMPI_Request* request) {
  int rc=0;
  assert(0);
  return rc;
}

/**
 * Tangent Isend, with separate shadow (i.e. tangent) buffer.
 */
int TLS_AMPI_Isend (void* buf, void* shadowbuf,
                    int count,
                    MPI_Datatype datatype, MPI_Datatype shadowdatatype,
                    int dest,
                    int tag,
                    AMPI_PairedWith pairedWith,
                    MPI_Comm comm,
                    AMPI_Request* request) {
  int rc = 0 ;
  MPI_Comm shadowcomm ;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  ampiRequest->plainRequest=*request;
#else 
  ampiRequest=request;
#endif
  /* fill in the other info. [llh]:I need none of those... */
  ampiRequest->endPoint=dest;
  ampiRequest->tag=tag;
  ampiRequest->count=count;
  ampiRequest->datatype=datatype;
  ampiRequest->comm=comm;
  ampiRequest->origin=AMPI_SEND_ORIGIN;
  ampiRequest->pairedWith=pairedWith;
  (*ourADTOOL_AMPI_FPCollection.mapBufForAdjoint_fp)(ampiRequest,shadowbuf);
  ampiRequest->tracedRequest=ampiRequest->plainRequest;
  rc = MPI_Isend(buf, count, datatype, dest, tag, comm,
                     &(ampiRequest->plainRequest)) ;
  assert(rc==MPI_SUCCESS);
  shadowcomm = (*ourADTOOL_AMPI_FPCollection.getShadowComm_fp)(comm) ;
  rc = MPI_Isend(shadowbuf, count, shadowdatatype, dest, tag, shadowcomm,
                 &(ampiRequest->shadowRequest)) ;
#ifdef AMPI_FORTRANCOMPATIBLE
  *request = ampiRequest->plainRequest ;
  BK_AMPI_put_AMPI_Request(ampiRequest);
#endif
  return rc ;
}

int FW_AMPI_Wait(AMPI_Request *request,
		 MPI_Status *status) { 
  int rc=0;
  MPI_Request *plainRequest;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  plainRequest=request;
  /*[llh] doubt about the 3rd argument (0?) for the OO traced case: */
  BK_AMPI_get_AMPI_Request(plainRequest,ampiRequest,0);
#else 
  plainRequest=&(request->plainRequest);
  ampiRequest=request;
#endif 
  rc=MPI_Wait(plainRequest,
	      status);
  if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(ampiRequest->datatype)==AMPI_ACTIVE) {
    (*ourADTOOL_AMPI_FPCollection.writeData_fp)(ampiRequest->buf,&ampiRequest->count);
    if(ampiRequest->tag==MPI_ANY_TAG) ampiRequest->tag=status->MPI_TAG;
    if(ampiRequest->endPoint==MPI_ANY_SOURCE) ampiRequest->endPoint=status->MPI_SOURCE;
    (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_WAIT);
    (*ourADTOOL_AMPI_FPCollection.push_AMPI_Request_fp)(ampiRequest);
  }
  return rc;
}

int BW_AMPI_Wait(AMPI_Request *request,
		 MPI_Status *status) {
  int rc=0;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
#else 
  ampiRequest=request;
#endif 
  /* pop request  */
  (*ourADTOOL_AMPI_FPCollection.pop_AMPI_Request_fp)(ampiRequest);
  switch(ampiRequest->origin) { 
  case AMPI_SEND_ORIGIN: { 
    (*ourADTOOL_AMPI_FPCollection.setAdjointCountAndTempBuf_fp)(ampiRequest);   
    rc=MPI_Irecv(ampiRequest->adjointTempBuf,
		 ampiRequest->adjointCount,
		 ampiRequest->datatype,
		 ampiRequest->endPoint,
		 ampiRequest->tag,
		 ampiRequest->comm,
		 &(ampiRequest->plainRequest));
    break;
  }
  case AMPI_RECV_ORIGIN: { 
    (*ourADTOOL_AMPI_FPCollection.setAdjointCount_fp)(ampiRequest);
    rc=MPI_Isend((*ourADTOOL_AMPI_FPCollection.rawAdjointData_fp)(ampiRequest->adjointBuf),
		 ampiRequest->adjointCount,
		 ampiRequest->datatype,
		 ampiRequest->endPoint,
		 ampiRequest->tag,
		 ampiRequest->comm,
		 &(ampiRequest->plainRequest));
    break;
  }
  default:  
    rc=MPI_Abort(ampiRequest->comm, MPI_ERR_TYPE);
    break;
  }
#ifdef AMPI_FORTRANCOMPATIBLE 
  *request=ampiRequest->plainRequest;
#endif
#if defined AMPI_FORTRANCOMPATIBLE || defined AMPI_REQUESTONTRACE
  BK_AMPI_put_AMPI_Request(ampiRequest);
#endif
  return rc;
}

int TLM_AMPI_Wait(AMPI_Request *request,
                  MPI_Status *status) {
  int rc=0;
  assert(0);
  return rc;
}

/**
 * Tangent Wait, with separate shadow (i.e. tangent) buffer.
 */
int TLS_AMPI_Wait(AMPI_Request *request,
                  MPI_Status *status) {
  int rc=0;
  MPI_Status status1 ;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  /*[llh] doubt about the 3rd argument (0?) for the OO traced case: */
  BK_AMPI_get_AMPI_Request(request,ampiRequest,0);
#else 
  ampiRequest=request;
#endif 
  rc=MPI_Wait(&(ampiRequest->plainRequest), &status1);
  assert(rc==MPI_SUCCESS);
  switch(ampiRequest->origin) { 
  case AMPI_SEND_ORIGIN: { 
    rc=MPI_Wait(&(ampiRequest->shadowRequest), status);
    break ;
  }
  case AMPI_RECV_ORIGIN: { 
    MPI_Comm shadowcomm = (*ourADTOOL_AMPI_FPCollection.getShadowComm_fp)(ampiRequest->comm) ;
    rc = MPI_Recv(ampiRequest->adjointBuf, ampiRequest->count, ampiRequest->datatype,
                  (ampiRequest->endPoint==MPI_ANY_SOURCE?status1.MPI_SOURCE:ampiRequest->endPoint),
                  (ampiRequest->tag==MPI_ANY_TAG?status1.MPI_TAG:ampiRequest->tag),
                  shadowcomm, status) ;
    break ;
  }
  default:
    rc=MPI_Abort(ampiRequest->comm, MPI_ERR_ARG);
    break ;
  }
  return rc;
}

int FW_AMPI_Barrier(MPI_Comm comm){
  int rc=0;
  rc=MPI_Barrier(comm);
  (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_BARRIER);
  (*ourADTOOL_AMPI_FPCollection.push_comm_fp)(comm);
  return rc;
}

int BW_AMPI_Barrier(MPI_Comm comm){
  int rc;
  comm=(*ourADTOOL_AMPI_FPCollection.pop_comm_fp)();
  rc=MPI_Barrier(comm);
  return rc;
}

int TLM_AMPI_Barrier(MPI_Comm comm){
  int rc=0;
  assert(0);
  return rc;
}

int TLS_AMPI_Barrier(MPI_Comm comm){
  int rc=0;
  rc=MPI_Barrier(comm);
  assert(rc==MPI_SUCCESS);
  MPI_Comm shadowcomm = (*ourADTOOL_AMPI_FPCollection.getShadowComm_fp)(comm) ;
  rc=MPI_Barrier(shadowcomm);
  return rc;
}

int FW_AMPI_Gather(void *sendbuf,
		   int sendcnt,
		   MPI_Datatype sendtype,
		   void *recvbuf,
		   int recvcnt,
		   MPI_Datatype recvtype,
		   int root,
		   MPI_Comm comm) {
  void *rawSendBuf=sendbuf, *rawRecvBuf=recvbuf;
  int rc=MPI_SUCCESS;
  int isInPlace=(sendbuf==MPI_IN_PLACE);
  int myRank, myCommSize;
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &myCommSize);
  if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)!=(*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)) {
    rc=MPI_Abort(comm, MPI_ERR_ARG);
  }
  else {
    if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE)  rawSendBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sendbuf,&sendcnt);
    if (myRank==root) {
      if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE)  rawRecvBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(recvbuf,&recvcnt);
    }
    rc=MPI_Gather(rawSendBuf,
		  sendcnt,
		  sendtype,
		  rawRecvBuf,
		  recvcnt,
		  recvtype,
		  root,
		  comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE) {
      if (myRank==root) (*ourADTOOL_AMPI_FPCollection.writeData_fp)(recvbuf,&recvcnt);
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_GATHER);
      (*ourADTOOL_AMPI_FPCollection.pushGSinfo_fp)(((myRank==root)?myCommSize:0),
						   recvbuf,
						   recvcnt,
						   recvtype,
						   sendbuf,
						   sendcnt,
						   sendtype,
						   root,
						   comm);
    }
  }
  return rc;
}

int BW_AMPI_Gather(void *sendbuf,
		   int sendcnt,
		   MPI_Datatype sendtype,
		   void *recvbuf,
		   int recvcnt,
		   MPI_Datatype recvtype,
		   int root,
		   MPI_Comm comm) {
  void *idx=NULL;
  int rc=MPI_SUCCESS;
  int commSizeForRootOrNull, rTypeSize,i;
  (*ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp)(&commSizeForRootOrNull);
  (*ourADTOOL_AMPI_FPCollection.popGSinfo_fp)(commSizeForRootOrNull,
					      &recvbuf,
					      &recvcnt,
					      &recvtype,
					      &sendbuf,
					      &sendcnt,
					      &sendtype,
					      &root,
					      &comm);
  (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&sendcnt,sendtype);
  void *tempBuf = 0;
  if (sendcnt>0) tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(sendcnt,sendtype,comm) ;
  else {
    if (commSizeForRootOrNull) 
      tempBuf=MPI_IN_PLACE;
    else 
      tempBuf=0;
  }
  rc=MPI_Scatter(recvbuf,
		 recvcnt,
		 recvtype,
		 tempBuf,
		 sendcnt,
		 sendtype,
		 root,
		 comm);
  (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(sendcnt,
						     sendtype,
						     comm,
						     sendbuf,
						     tempBuf, idx);
  if (commSizeForRootOrNull) {
    MPI_Type_size(recvtype,&rTypeSize);
    for (i=0;i<commSizeForRootOrNull;++i) { 
      if (! (i==root && sendcnt==0)) { /* don't nullify the segment if "in place" on root */
	void *recvbufSegment=(char*)recvbuf+(i*recvcnt*rTypeSize);
	(*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(recvcnt,recvtype,comm,
							 recvbufSegment);
      }
    }
  }
  if (tempBuf!=MPI_IN_PLACE && tempBuf!=0) (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  return rc;
}

int TLM_AMPI_Gather(void *sendbuf,
                    int sendcnt,
                    MPI_Datatype sendtype,
                    void *recvbuf,
                    int recvcnt,
                    MPI_Datatype recvtype,
                    int root,
                    MPI_Comm comm) {
  int rc;
  rc = MPI_Gather(sendbuf,sendcnt,sendtype,recvbuf,recvcnt,recvtype,root,comm);
  return rc;
}

int FW_AMPI_Scatter(void *sendbuf,
                     int sendcnt,
                     MPI_Datatype sendtype,
                     void *recvbuf,
                     int recvcnt,
                     MPI_Datatype recvtype,
                     int root,
                     MPI_Comm comm) {
  int rc=MPI_SUCCESS;
  int myRank, myCommSize;
  int isInPlace=(recvbuf==MPI_IN_PLACE);
  void *rawSendBuf=sendbuf, *rawRecvBuf=recvbuf;
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &myCommSize);
  if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)!=(*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)) {
    rc=MPI_Abort(comm, MPI_ERR_ARG);
  }
  else {
    if (myRank==root) {
      if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE)  rawSendBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sendbuf,&sendcnt);
    }
    if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE)  rawRecvBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(recvbuf,&recvcnt);
    rc=MPI_Scatter(rawSendBuf,
                   sendcnt,
                   sendtype,
                   rawRecvBuf,
                   recvcnt,
                   recvtype,
                   root,
                   comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.writeData_fp)(recvbuf,&recvcnt);
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_SCATTER);
      (*ourADTOOL_AMPI_FPCollection.pushGSinfo_fp)(((myRank==root)?myCommSize:0),
						   sendbuf,
						   sendcnt,
						   sendtype,
						   recvbuf,
						   recvcnt,
						   recvtype,
						   root,
						   comm);
    }
  }
  return rc;
}

int BW_AMPI_Scatter(void *sendbuf,
                     int sendcnt,
                     MPI_Datatype sendtype,
                     void *recvbuf,
                     int recvcnt,
                     MPI_Datatype recvtype,
                     int root,
                     MPI_Comm comm) {
  int rc=MPI_SUCCESS;
  void *idx=NULL;
  int commSizeForRootOrNull,i,rTypeSize;
  (*ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp)(&commSizeForRootOrNull);
  (*ourADTOOL_AMPI_FPCollection.popGSinfo_fp)(commSizeForRootOrNull,
					      &sendbuf,
					      &sendcnt,
					      &sendtype,
					      &recvbuf,
					      &recvcnt,
					      &recvtype,
					      &root,
					      &comm);
  void *tempBuf = NULL;
  if (commSizeForRootOrNull>0) tempBuf=(*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(sendcnt*commSizeForRootOrNull,sendtype,comm);
  rc=MPI_Gather(recvbuf,
		recvcnt,
		recvtype,
		tempBuf,
                sendcnt,
		sendtype,
		root,
		comm);
  (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(recvcnt,recvtype,comm, recvbuf);
  if (commSizeForRootOrNull>0) MPI_Type_size(recvtype,&rTypeSize);
  for (i=0;i<commSizeForRootOrNull;++i) {
    if (! (i==root && recvcnt==0)) { /* don't increment the segment if "in place" on root */
      void *tempBufSeqment=(char*)tempBuf+i*sendcnt*rTypeSize;
      void *sendBufSegment=(char*)sendbuf+i*sendcnt*rTypeSize;
      (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(sendcnt,
                                                         sendtype,
                                                         comm,
                                                         sendBufSegment,
                                                         tempBufSeqment, idx);
    }
  }
  if (commSizeForRootOrNull>0 && tempBuf)(*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  return rc;
}

int TLM_AMPI_Scatter(void *sendbuf,
                     int sendcnt,
                     MPI_Datatype sendtype,
                     void *recvbuf,
                     int recvcnt,
                     MPI_Datatype recvtype,
                     int root,
                     MPI_Comm comm){
  int rc;
  rc = MPI_Scatter(sendbuf,sendcnt,sendtype,recvbuf,recvcnt,recvtype,root,comm);
  return rc;
}

int FW_AMPI_Allgather(void *sendbuf,
                      int sendcount,
                      MPI_Datatype sendtype,
                      void *recvbuf,
                      int recvcount,
                      MPI_Datatype recvtype,
                      MPI_Comm comm) {
  void *rawSendBuf=NULL, *rawRecvBuf=NULL;
  int rc=MPI_SUCCESS;
  int myRank, myCommSize;
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &myCommSize);
  if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)!=(*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)) {
    rc=MPI_Abort(comm, MPI_ERR_ARG);
  }
  else {
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE)  rawSendBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sendbuf,&sendcount);
    else rawSendBuf=sendbuf;
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE)  rawRecvBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(recvbuf,&recvcount);
    else rawRecvBuf=recvbuf;
    rc=MPI_Allgather(rawSendBuf,
                     sendcount,
                     sendtype,
                     rawRecvBuf,
                     recvcount,
                     recvtype,
                     comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.writeData_fp)(recvbuf,&recvcount);
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_ALLGATHER);
      (*ourADTOOL_AMPI_FPCollection.pushGSinfo_fp)((myCommSize),
						   recvbuf,
						   recvcount,
						   recvtype,
						   sendbuf,
						   sendcount,
						   sendtype,
						   0,
						   comm);
    }
  }
  return rc;
}

int BW_AMPI_Allgather(void *sendbuf,
                      int sendcount,
                      MPI_Datatype sendtype,
                      void *recvbuf,
                      int recvcount,
                      MPI_Datatype recvtype,
                      MPI_Comm comm) {
  void *idx=NULL;
  int rc=MPI_SUCCESS, rootPlaceholder;
  int commSizeForRootOrNull, rTypeSize, *recvcounts,i;
  (*ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp)(&commSizeForRootOrNull);
  (*ourADTOOL_AMPI_FPCollection.popGSinfo_fp)(commSizeForRootOrNull,
					      &recvbuf,
					      &recvcount,
					      &recvtype,
					      &sendbuf,
					      &sendcount,
					      &sendtype,
					      &rootPlaceholder,
					      &comm);
  recvcounts=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
  for (i=0;i<commSizeForRootOrNull;++i) recvcounts[i]=sendcount;
  void *tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(sendcount,sendtype,comm);
  /**
   * \todo shortcut taken below by assuming AMPI_ADOUBLE is equivalent to MPI_DOUBLE, need type map.
   */
  rc=MPI_Reduce_scatter(recvbuf,
                        tempBuf,
                        recvcounts,
                        MPI_DOUBLE, /* <<< here is the offending bit */
                        MPI_SUM,
                        comm);
  (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(sendcount,
                                                     sendtype,
                                                     comm,
                                                     sendbuf,
                                                     tempBuf, idx);
  if (commSizeForRootOrNull) {
    MPI_Type_size(recvtype,&rTypeSize);
    (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(recvcount*commSizeForRootOrNull,
                                                     recvtype,comm,recvbuf);
  }
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  if (recvcounts) free((void*)recvcounts);
  return rc;
}

int TLM_AMPI_Allgather(void *sendbuf,
                       int sendcount,
                       MPI_Datatype sendtype,
                       void *recvbuf,
                       int recvcount,
                       MPI_Datatype recvtype,
                       MPI_Comm comm) {
  int rc;
  rc = MPI_Allgather(sendbuf,sendcount,sendtype,recvbuf,recvcount,recvtype,comm);
  return rc;
}

int FW_AMPI_Gatherv(void *sendbuf,
                    int sendcnt,
                    MPI_Datatype sendtype,
                    void *recvbuf,
                    int *recvcnts,
                    int *displs,
                    MPI_Datatype recvtype,
                    int root,
                    MPI_Comm comm) {
  void *rawSendBuf=sendbuf, *rawRecvBuf=recvbuf;
  int rc=MPI_SUCCESS;
  int isInPlace=(sendbuf==MPI_IN_PLACE);
  int myRank, myCommSize;
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &myCommSize);
  if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)!=(*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)) {
    rc=MPI_Abort(comm, MPI_ERR_ARG);
  }
  else {
    if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE)  rawSendBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sendbuf,&sendcnt);
    if (myRank==root) {
      if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE)  rawRecvBuf=(*ourADTOOL_AMPI_FPCollection.rawDataV_fp)(recvbuf, myCommSize, recvcnts, displs);
    }
    rc=MPI_Gatherv(rawSendBuf,
                   sendcnt,
                   sendtype,
                   rawRecvBuf,
                   recvcnts,
                   displs,
                   recvtype,
                   root,
                   comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE) {
      if (myRank==root) (*ourADTOOL_AMPI_FPCollection.writeDataV_fp)(recvbuf,recvcnts, displs);
      (*ourADTOOL_AMPI_FPCollection.push_CallCodeReserve_fp)(AMPI_GATHERV,((myRank==root)?myCommSize:0)*2);
      (*ourADTOOL_AMPI_FPCollection.pushGSVinfo_fp)(((myRank==root)?myCommSize:0),
						    recvbuf,
						    recvcnts,
						    displs,
						    recvtype,
						    sendbuf,
						    sendcnt,
						    sendtype,
						    root,
						    comm);
    }
  }
  return rc;
}

int BW_AMPI_Gatherv(void *sendbuf,
                    int sendcnt,
                    MPI_Datatype sendtype,
                    void *recvbuf,
                    int *recvcnts,
                    int *displs,
                    MPI_Datatype recvtype,
                    int root,
                    MPI_Comm comm) {
  void *idx=NULL;
  int i;
  int rc=MPI_SUCCESS;
  int myRank, commSizeForRootOrNull, rTypeSize;
  int *tRecvCnts=recvcnts, *tDispls=displs;
  char tRecvCntsFlag=0, tDisplsFlag=0;
  (*ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp)(&commSizeForRootOrNull);
  if (tRecvCnts==NULL) {
    tRecvCnts=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
    tRecvCntsFlag=1;
  }
  if (tDispls==NULL) {
    tDispls=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
    tDisplsFlag=1;
  }
  (*ourADTOOL_AMPI_FPCollection.popGSVinfo_fp)(commSizeForRootOrNull,
					       &recvbuf,
					       tRecvCnts,
					       tDispls,
					       &recvtype,
					       &sendbuf,
					       &sendcnt,
					       &sendtype,
					       &root,
					       &comm);
  MPI_Comm_rank(comm, &myRank);
  (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&sendcnt,sendtype);
  void *tempBuf = 0;
  if (sendcnt>0) tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(sendcnt,sendtype,comm) ;
  else {
    if (commSizeForRootOrNull) 
      tempBuf=MPI_IN_PLACE;
    else 
      tempBuf=0;
  }
  rc=MPI_Scatterv(recvbuf,
                  tRecvCnts,
                  tDispls,
                  recvtype,
                  tempBuf,
                  sendcnt,
                  sendtype,
                  root,
                  comm);
  (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(sendcnt,
						     sendtype,
						     comm,
						     sendbuf,
						     tempBuf, idx);
  if (commSizeForRootOrNull) {
    MPI_Type_size(recvtype,&rTypeSize);
    for (i=0;i<commSizeForRootOrNull;++i) {
      if (! (i==root && sendcnt==0)) { /* don't nullify the segment if "in place" on root */
	void* recvbufSegment=(char*)recvbuf+(rTypeSize*tDispls[i]); /* <----------  very iffy! */
	(*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(tRecvCnts[i],recvtype,comm,
							 recvbufSegment);
      }
    }
  }
  if (tempBuf!=MPI_IN_PLACE && tempBuf!=0) (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  if (tRecvCntsFlag) free((void*)(tRecvCnts));
  if (tDisplsFlag) free((void*)(tDispls));
  return rc;
}

int TLM_AMPI_Gatherv(void *sendbuf,
                     int sendcnt,
                     MPI_Datatype sendtype,
                     void *recvbuf,
                     int *recvcnts,
                     int *displs,
                     MPI_Datatype recvtype,
                     int root,
                     MPI_Comm comm) {
  int rc;
  rc = MPI_Gatherv(sendbuf,sendcnt,sendtype,recvbuf,recvcnts,displs,recvtype,root,comm);
  return rc;
}

int FW_AMPI_Scatterv(void *sendbuf,
                     int *sendcnts,
                     int *displs,
                     MPI_Datatype sendtype,
                     void *recvbuf,
                     int recvcnt,
                     MPI_Datatype recvtype,
                     int root,
                     MPI_Comm comm) {
  int rc=MPI_SUCCESS;
  int myRank, myCommSize;
  int isInPlace=(recvbuf==MPI_IN_PLACE);
  void *rawSendBuf=sendbuf, *rawRecvBuf=recvbuf;
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &myCommSize);
  if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)!=(*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)) {
    rc=MPI_Abort(comm, MPI_ERR_ARG);
  }
  else {
    if (myRank==root) {
      if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE) rawSendBuf=(*ourADTOOL_AMPI_FPCollection.rawDataV_fp)(sendbuf,myCommSize,sendcnts,displs);
    }
    if (!isInPlace && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE)  rawRecvBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(recvbuf,&recvcnt);
    rc=MPI_Scatterv(rawSendBuf,
                    sendcnts,
                    displs,
                    sendtype,
                    rawRecvBuf,
                    recvcnt,
                    recvtype,
                    root,
                    comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.writeData_fp)(recvbuf,&recvcnt);
      (*ourADTOOL_AMPI_FPCollection.push_CallCodeReserve_fp)(AMPI_SCATTERV,((myRank==root)?myCommSize:0)*2);
      (*ourADTOOL_AMPI_FPCollection.pushGSVinfo_fp)(((myRank==root)?myCommSize:0),
						    sendbuf,
						    sendcnts,
						    displs,
						    sendtype,
						    recvbuf,
						    recvcnt,
						    recvtype,
						    root,
						    comm);
    }
  }
  return rc;
}

int BW_AMPI_Scatterv(void *sendbuf,
                     int *sendcnts,
                     int *displs,
                     MPI_Datatype sendtype,
                     void *recvbuf,
                     int recvcnt,
                     MPI_Datatype recvtype,
                     int root,
                     MPI_Comm comm) {
  int rc=MPI_SUCCESS;
  void *idx=NULL;
  int sendSize=0,i, typeSize;
  int myRank, commSizeForRootOrNull, *tempDispls;
  int *tSendCnts=sendcnts, *tDispls=displs;
  char tSendCntsFlag=0, tDisplsFlag=0;
  (*ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp)(&commSizeForRootOrNull);
  if (tSendCnts==NULL && commSizeForRootOrNull>0) {
    tSendCnts=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
    tSendCntsFlag=1;
  }
  if (tDispls==NULL && commSizeForRootOrNull>0) {
    tDispls=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
    tDisplsFlag=1;
  }
  (*ourADTOOL_AMPI_FPCollection.popGSVinfo_fp)(commSizeForRootOrNull,
					       &sendbuf,
					       tSendCnts,
					       tDispls,
					       &sendtype,
					       &recvbuf,
					       &recvcnt,
					       &recvtype,
					       &root,
					       &comm);
  MPI_Comm_rank(comm, &myRank);
  tempDispls=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
  for (i=0;i<commSizeForRootOrNull;++i) {
    tempDispls[i]=sendSize;
    sendSize+=tSendCnts[i];
  }
  void *tempBuf = NULL;
  if (commSizeForRootOrNull>0) tempBuf=(*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(sendSize,sendtype,comm);
  rc=MPI_Gatherv(recvbuf,
                 recvcnt,
                 recvtype,
                 tempBuf,
                 tSendCnts,
                 tempDispls,
                 sendtype,
                 root,
                 comm);
  (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(recvcnt,recvtype,comm,recvbuf);
  if (commSizeForRootOrNull>0) {
    MPI_Type_size(sendtype,&typeSize);
    for (i=0;i<commSizeForRootOrNull;++i) {
      if (! (i==root && recvcnt==0)) { /* don't increment the segment if "in place" on root */
        void* buf=(char*)sendbuf+(typeSize*tDispls[i]); /* <----------  very iffy! */
        void* sourceBuf=(char*)tempBuf+(typeSize*tempDispls[i]);
        (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(tSendCnts[i],
                                                           sendtype,
                                                           comm,
                                                           buf,
                                                           sourceBuf, idx);
      }
    }
    (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  }
  if (tempDispls) free((void*)tempDispls);
  if (tSendCntsFlag) free((void*)(tSendCnts));
  if (tDisplsFlag) free((void*)(tDispls));
  return rc;
}

int TLM_AMPI_Scatterv(void *sendbuf,
                      int *sendcnts,
                      int *displs,
                      MPI_Datatype sendtype,
                      void *recvbuf,
                      int recvcnt,
                      MPI_Datatype recvtype,
                      int root, MPI_Comm comm){
  int rc;
  rc = MPI_Scatterv(sendbuf,sendcnts,displs,sendtype,recvbuf,recvcnt,recvtype,root,comm);
  return rc;
}

int FW_AMPI_Allgatherv(void *sendbuf,
                       int sendcnt,
                       MPI_Datatype sendtype,
                       void *recvbuf,
                       int *recvcnts,
                       int *displs,
                       MPI_Datatype recvtype,
                       MPI_Comm comm) {
  void *rawSendBuf=NULL, *rawRecvBuf=NULL;
  int rc=MPI_SUCCESS;
  int myRank, myCommSize;
  MPI_Comm_rank(comm, &myRank);
  MPI_Comm_size(comm, &myCommSize);
  if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)!=(*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)) {
    rc=MPI_Abort(comm, MPI_ERR_ARG);
  }
  else {
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(sendtype)==AMPI_ACTIVE)  rawSendBuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sendbuf,&sendcnt);
    else rawSendBuf=sendbuf;
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE)  rawRecvBuf=(*ourADTOOL_AMPI_FPCollection.rawDataV_fp)(recvbuf, myCommSize, recvcnts, displs);
    else rawRecvBuf=recvbuf;
    rc=MPI_Allgatherv(rawSendBuf,
                      sendcnt,
                      sendtype,
                      rawRecvBuf,
                      recvcnts,
                      displs,
                      recvtype,
                      comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(recvtype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.writeDataV_fp)(recvbuf,recvcnts, displs);
      (*ourADTOOL_AMPI_FPCollection.push_CallCodeReserve_fp)(AMPI_ALLGATHERV,myCommSize*2);
      (*ourADTOOL_AMPI_FPCollection.pushGSVinfo_fp)(myCommSize,
						    recvbuf,
						    recvcnts,
						    displs,
						    recvtype,
						    sendbuf,
						    sendcnt,
						    sendtype,
						    0,
						    comm);
    }
  }
  return rc;
}

int BW_AMPI_Allgatherv(void *sendbuf,
                       int sendcnt,
                       MPI_Datatype sendtype,
                       void *recvbuf,
                       int *recvcnts,
                       int *displs,
                       MPI_Datatype recvtype,
                       MPI_Comm comm) {
  void *idx=NULL;
  int i;
  int rc=MPI_SUCCESS;
  int myRank, commSizeForRootOrNull, rTypeSize,rootPlaceholder;
  int *tRecvCnts=recvcnts, *tDispls=displs;
  char tRecvCntsFlag=0, tDisplsFlag=0;
  (*ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp)(&commSizeForRootOrNull);
  if (tRecvCnts==NULL) {
    tRecvCnts=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
    tRecvCntsFlag=1;
  }
  if (tDispls==NULL) {
    tDispls=(int*)malloc(sizeof(int)*commSizeForRootOrNull);
    tDisplsFlag=1;
  }
  (*ourADTOOL_AMPI_FPCollection.popGSVinfo_fp)(commSizeForRootOrNull,
					       &recvbuf,
					       tRecvCnts,
					       tDispls,
					       &recvtype,
					       &sendbuf,
					       &sendcnt,
					       &sendtype,
					       &rootPlaceholder,
					       &comm);
  MPI_Comm_rank(comm, &myRank);
  void *tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(tRecvCnts[myRank],sendtype,comm) ;
  /**
   * \todo shortcut taken below by assuming AMPI_ADOUBLE is equivalent to MPI_DOUBLE, need type map.
   */
  rc=MPI_Reduce_scatter(recvbuf,
                        tempBuf,
                        tRecvCnts,
                        MPI_DOUBLE, /* <<< here is the offending bit */
                        MPI_SUM,
                        comm);
  (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(sendcnt,
                                                     sendtype,
                                                     comm,
                                                     sendbuf,
                                                     tempBuf, idx);
  MPI_Type_size(recvtype,&rTypeSize);
  for (i=0;i<commSizeForRootOrNull;++i) {
    void* buf=(char*)recvbuf+(rTypeSize*tDispls[i]); /* <----------  very iffy! */
    (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(tRecvCnts[i],recvtype,comm,buf);
  }
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  if (tRecvCntsFlag) free((void*)(tRecvCnts));
  if (tDisplsFlag) free((void*)(tDispls));
  return rc;
}

int TLM_AMPI_Allgatherv(void *sendbuf,
                        int sendcnt,
                        MPI_Datatype sendtype,
                        void *recvbuf,
                        int *recvcnts,
                        int *displs,
                        MPI_Datatype recvtype,
                        MPI_Comm comm) {
  int rc;
  rc = MPI_Allgatherv(sendbuf,sendcnt,sendtype,recvbuf,recvcnts,displs,recvtype,comm);
  return rc;
}

int FW_AMPI_Bcast (void* buf,
                   int count,
                   MPI_Datatype datatype,
                   int root,
                   MPI_Comm comm) {
  int rc=0;
  double* mappedbuf=NULL;
  int dt_idx = derivedTypeIdx(datatype);
  int is_derived = isDerivedType(dt_idx);
  if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
    mappedbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(buf,&count);
  }
  else if(is_derived) {
    mappedbuf=(*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,datatype,comm);
    (*ourADTOOL_AMPI_FPCollection.packDType_fp)(buf,mappedbuf,count,dt_idx);
  }
  else {
    mappedbuf=buf;
  }
  rc=MPI_Bcast(mappedbuf,
               count,
               (*ourADTOOL_AMPI_FPCollection.FW_rawType_fp)(datatype),
               root,
               comm);
  if (rc==MPI_SUCCESS && ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE || is_derived )) {
    if (is_derived) {
      (*ourADTOOL_AMPI_FPCollection.unpackDType_fp)(mappedbuf,buf,count,dt_idx);
      (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(mappedbuf);
    }
    (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_BCAST);
    (*ourADTOOL_AMPI_FPCollection.pushBcastInfo_fp)(buf,
						    count,
						    datatype,
						    root,
						    comm);
  }
  return rc;
}

int BW_AMPI_Bcast (void* buf,
                   int count,
                   MPI_Datatype datatype,
                   int root,
                   MPI_Comm comm) {
  int rc,rank;
  void *idx=NULL;
  (*ourADTOOL_AMPI_FPCollection.popBcastInfo_fp)(&buf,
						 &count,
						 &datatype,
						 &root,
						 &comm,
						 &idx);
  MPI_Comm_rank(comm,&rank);
  MPI_Datatype mappedtype = (*ourADTOOL_AMPI_FPCollection.BW_rawType_fp)(datatype);
  (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&count,datatype);
  void *tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,datatype,comm);
  rc=MPI_Reduce(buf,
                tempBuf,
                count,
                mappedtype,
                MPI_SUM,
                root,
                comm);
  (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count, mappedtype, comm, buf);
  if (rank==root) {
    (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count, mappedtype, comm,
                                                       buf, tempBuf, idx);
  }
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  return rc;
}

int TLM_AMPI_Bcast(void* buf,
                   int count,
                   MPI_Datatype datatype,
                   int root,
                   MPI_Comm comm){
  int rc=MPI_Bcast(buf,
                   count,
                   datatype,
                   root,
                   comm);
  return rc;
}

/** Pass split_mode 0 to obtain a joint adjoint reduction-driver and a joint adjoint reduction-op
 *  Pass split_mode 1 to obtain a split adjoint reduction-driver and a joint adjoint reduction-op
 *  Pass split_mode 2 to obtain a split adjoint reduction-driver and a split adjoint reduction-op
 *  When no adjoint is required (sbufb null), pass split_mode 0
 */
int PEDESTRIAN_AMPI_Reduce(void* sbuf, void* sbufd, void* sbufb,
                    void* rbuf, void* rbufd, void* rbufb,
                    int count,
                    MPI_Datatype datatype, MPI_Datatype datatyped, MPI_Datatype datatypeb,
                    MPI_Op op, TLM_userFunctionF* uopd, ADJ_userFunctionF* uopb,
                    int split_mode,
                    int root,
                    MPI_Comm comm) {
  if (count == 0) return MPI_SUCCESS;
  int rc, rank ;
  MPI_Comm_rank(comm,&rank) ;
  void *idx=NULL; /* only for compatibility in incrementAdjoint_fp(..., idx) */
  int reduceTgt = (sbufd!=NULL) ;
  int reduceAdj = (sbufb!=NULL) ;
  MPI_Comm shadowcomm = comm ;
  if (reduceTgt)
    shadowcomm = (*ourADTOOL_AMPI_FPCollection.getShadowComm_fp)(comm) ;

  if (uopd || uopb || op!=MPI_SUM) {

    int uop_idx = userDefinedOpIdx(op);
    userDefinedOpData* uopdata = (isUserDefinedOp(uop_idx)?getUOpData():NULL) ;
    int is_commutative = (uopdata?uopdata->commutes[uop_idx]:1) ;
    if (uopdata) {
      if (reduceTgt) assert(uopd) ;
      if (reduceAdj) assert(uopb) ;
    }

    void *exch_buf=NULL ;
    int switched = 0 ;

    void *obuf=NULL, *obufd=NULL ;

    int dt_idx = derivedTypeIdx(datatype);
    MPI_Aint lb = (isDerivedType(dt_idx)?getDTypeData()->lbs[dt_idx]:0) ;
    int dt_idxd = 0 ;
    MPI_Aint lbd = 0 ;
    obuf =
      (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatype,comm);
    obuf = (void*)((char*)obuf - lb);
    if (reduceTgt) {
      dt_idxd = derivedTypeIdx(datatyped);
      lbd = (isDerivedType(dt_idxd)?getDTypeData()->lbs[dt_idxd]:0) ;
      obufd =
        (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatyped,shadowcomm);
      obufd = (void*)((char*)obufd - lbd);
    }

    if (sbuf==MPI_IN_PLACE) {
      if (rank != root) {
        exch_buf = rbuf ;
        rbuf = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatype,comm);
        rbuf = (void*)((char*)rbuf - lb);
        (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(exch_buf, rbuf, count, datatype, comm);
        if (reduceTgt) {
          exch_buf = rbufd ;
          rbufd = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatyped,shadowcomm);
          rbufd = (void*)((char*)rbufd - lbd);
          (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(exch_buf, rbufd, count, datatyped, shadowcomm);
        }
      }
    } else {  /* Standard case: sbuf != MPI_IN_PLACE */
      if (rank != root) {
        rbuf = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatype,comm);
        rbuf = (void*)((char*)rbuf - lb);
        if (reduceTgt) {
          rbufd = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatyped,shadowcomm);
          rbufd = (void*)((char*)rbufd - lbd);
        }
      }
      (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(sbuf, rbuf, count, datatype, comm);
      if (reduceTgt)
        (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(sbufd, rbufd, count, datatyped, shadowcomm);
    }

    MPI_Status status;
    int comm_size ;
    MPI_Comm_size(comm,&comm_size);
    int other, action ;
    int maskup = 0xffffffff ;
    int mask   = 0x1;

    if (!reduceAdj || split_mode==0) {
     while (mask < comm_size) {
      if ((rank&mask) == 0) { /* Typical action is RECV */
        other = (rank==root?root&maskup:rank) | mask ;
        if (other >= comm_size)
          action = 0/*NOACTION*/ ;
        else if ((other&maskup) == (root&maskup)) {
          other = root ;
          action = 1/*SEND*/ ;
        } else {
          action = -1/*RECV*/ ;
        }
      } else { /* mask&rank == 1: Typical action is SEND */
        other = (rank==root?root&maskup:rank) & ~mask ;
        if ((other&maskup) == (root&maskup)) other = root ;
        if (rank==root)
          action = -1/*RECV*/ ;
        else
          action = 1/*SEND*/ ;
      }
      maskup = maskup & ~mask ;
      mask<<=1;

      if (action==1/*SEND*/) {
        /* TODO Not sure this "(..., 11, comm)" is correct. Would better use shadowcomm ? */
        rc = MPI_Send(rbuf, count, datatype, other, 11, comm) ;
        assert(rc==MPI_SUCCESS);
        if (reduceTgt) {
          rc = MPI_Send(rbufd, count, datatyped, other, 11, shadowcomm) ;
          assert(rc==MPI_SUCCESS);
        }
	break;
      } else if (action==-1/*RECV*/) {
        rc = MPI_Recv(obuf, count, datatype, other, 11, comm, &status);
        assert(rc==MPI_SUCCESS);
        if (reduceTgt) {
          rc = MPI_Recv(obufd, count, datatyped, other, 11, shadowcomm, &status);
          assert(rc==MPI_SUCCESS);
        }
        if (is_commutative || (other<rank)) {
          /* Save obuf and rbuf for future use in the adjoint sweep */
          (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(count,datatype,comm,obuf) ;
          if (split_mode!=2) {
            (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(count,datatype,comm,rbuf) ;
          }
          if (isUserDefinedOp(uop_idx)) {
            if (reduceTgt)
              (*uopd)(obuf, obufd, rbuf, rbufd, &count, &datatype, &datatyped);
            else
              (*(uopdata->functions[uop_idx]))(obuf, rbuf, &count, &datatype) ;
          } else {
            if (op==MPI_PROD) {
              (*ourADTOOL_AMPI_FPCollection.tangentMultiply_fp)
                (count, datatype, comm, obuf, obufd, rbuf, rbufd) ;
            } else if (op==MPI_MIN) {
              (*ourADTOOL_AMPI_FPCollection.tangentMin_fp)
                (count, datatype, comm, obuf, obufd, rbuf, rbufd) ;
            } else if (op==MPI_MAX) {
              (*ourADTOOL_AMPI_FPCollection.tangentMax_fp)
                (count, datatype, comm, obuf, obufd, rbuf, rbufd) ;
            } else {
              printf(__FILE__ ": tangent AMPI reduction not yet implemented for std op==%i\n",uop_idx) ;
            }
          }
        } else {
          /* Save obuf and rbuf for future use in the adjoint sweep */
          (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(count,datatype,comm,rbuf) ;
          if (split_mode!=2) {
            (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(count,datatype,comm,obuf) ;
          }
          if (reduceTgt)
            (*uopd)(rbuf, rbufd, obuf, obufd, &count, &datatype, &datatyped);
          else
            (*(uopdata->functions[uop_idx]))(rbuf, obuf, &count, &datatype) ;
          exch_buf = obuf ; obuf = rbuf ; rbuf = exch_buf ;
          if (reduceTgt) {
            exch_buf = obufd ; obufd = rbufd ; rbufd = exch_buf ;
          }
          switched = ~switched ;
        }
      }
     }

     if (switched) {
      if (!reduceAdj) { /* Adjoint joint mode does not need to return a correct rbuf */
        exch_buf = obuf ; obuf = rbuf ; rbuf = exch_buf ;
        if (rank==root)
          (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(obuf, rbuf, count, datatype, comm) ;
      }
      if (reduceTgt) {
        exch_buf = obufd ; obufd = rbufd ; rbufd = exch_buf ;
        if (rank==root)
          (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(obufd, rbufd, count, datatyped, shadowcomm) ;
      }
     }

    }

    if (split_mode!=0) {
      if (!reduceAdj) {
        (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(1,MPI_INT,comm,&switched) ;
        (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(1,MPI_INT,comm,&maskup) ;
        (*ourADTOOL_AMPI_FPCollection.pushBuffer_fp)(1,MPI_INT,comm,&mask) ;
      } else {
        (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(1,MPI_INT,comm,&mask) ;
        (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(1,MPI_INT,comm,&maskup) ;
        (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(1,MPI_INT,comm,&switched) ;
      }
    }

    if (reduceAdj) {
      void *rbufb_initial=NULL ;
      int dt_idxb = derivedTypeIdx(datatypeb);
      MPI_Aint lbb = (isDerivedType(dt_idxb)?getDTypeData()->lbs[dt_idxb]:0) ;
      void *obufb =
        (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatypeb,comm);
      obufb = (void*)((char*)obufb - lbb);
      (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,datatypeb,comm,obufb);
      if (rank != root) {
        rbufb_initial = rbufb ; 
        rbufb = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatypeb,comm);
        rbufb = (void*)((char*)rbufb - lbb);
        (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,datatypeb,comm,rbufb);
      }
      if (switched && rank==root) {
        (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(rbufb, obufb, count, datatypeb, comm) ;
        (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,datatypeb,comm,rbufb);
        exch_buf = obufb ; obufb = rbufb ; rbufb = exch_buf ;
      }
      while (mask!=0x1) {
        mask>>=1 ;
        maskup = maskup | mask ;
        if ((rank&mask) == 0) { /* Typical action fw is RECV */
          other = (rank==root?root&maskup:rank) | mask ;
          if (other >= comm_size)
            action = 0/*NOACTION*/ ;
          else if ((other&maskup) == (root&maskup)) {
            other = root ;
            action = 1/* fw SEND*/ ;
          } else {
            action = -1/* fw RECV*/ ;
          }
        } else { /* mask&rank == 1: Typical action is SEND */
          other = (rank==root?root&maskup:rank) & ~mask ;
          if ((other&maskup) == (root&maskup)) other = root ;
          if (rank==root)
            action = -1/* fw RECV*/ ;
          else
            action = 1/* fw SEND*/ ;
        }

        if (action==1/* fw SEND*/) {
          rc = MPI_Recv(obufb, count, datatypeb, other, 11, comm, &status);
          assert(rc==MPI_SUCCESS);
          (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count,datatypeb,comm,rbufb,obufb, idx) ;
        } else if (action==-1/* fw RECV*/) {
          if (is_commutative || (other<rank)) {
            /* Retrieve obuf and rbuf for the adjoint call */
            if (split_mode!=2) {
              (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(count,datatype,comm,rbuf) ;
            }
            (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(count,datatype,comm,obuf) ;
            (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,datatypeb,comm,obufb);
            if (isUserDefinedOp(uop_idx)) {
              (*uopb)(obuf, obufb, rbuf, rbufb, &count, &datatype, &datatypeb) ;
            } else {
              if (op==MPI_PROD) {
                (*ourADTOOL_AMPI_FPCollection.adjointMultiply_fp)
                  (count, datatype, comm, obuf, obufb, rbuf, rbufb) ;
              } else if (op==MPI_MIN) {
                (*ourADTOOL_AMPI_FPCollection.adjointMin_fp)
                  (count, datatype, comm, obuf, obufb, rbuf, rbufb) ;
              } else if (op==MPI_MAX) {
                (*ourADTOOL_AMPI_FPCollection.adjointMax_fp)
                  (count, datatype, comm, obuf, obufb, rbuf, rbufb) ;
              } else {
                printf(__FILE__ ": adjoint AMPI reduction not yet implemented for std op==%i\n",uop_idx) ;
              }
            }
          } else {
            exch_buf = obuf ; obuf = rbuf ; rbuf = exch_buf ;
            exch_buf = obufb ; obufb = rbufb ; rbufb = exch_buf ;
            /* Retrieve obuf and rbuf for the adjoint call */
            if (split_mode!=2) {
              (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(count,datatype,comm,obuf) ;
            }
            (*ourADTOOL_AMPI_FPCollection.popBuffer_fp)(count,datatype,comm,rbuf) ;
            (*uopb)(rbuf, rbufb, obuf, obufb, &count, &datatype, &datatypeb) ;
          }
          rc = MPI_Send(obufb, count, datatypeb, other, 11, comm);
          assert(rc==MPI_SUCCESS);
        }
        (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,datatypeb,comm,obufb);
      }
      if (sbuf==MPI_IN_PLACE) {
        if (rank != root) {
          (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count,datatypeb,comm,rbufb_initial,rbufb, idx) ;
        }
      } else {
        (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count,datatypeb,comm,sbufb,rbufb, idx) ;
        (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count,datatypeb,comm,rbufb);
      }
      if (rank!=root) (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(rbufb,count,datatypeb);
      (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(obufb,count,datatypeb);
    }

    (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(obuf,count,datatype);
    if (reduceTgt) (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(obufd,count,datatyped);
    if (rank!=root) {
      (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(rbuf,count,datatype);
      if (reduceTgt) (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(rbufd,count,datatyped);
    }
    return MPI_SUCCESS;
  } else {  /* i.e. op==MPI_SUM and no user-given derivative */
    if (!reduceAdj) {
      rc=MPI_Reduce(sbuf,
                    rbuf,
                    count,
                    datatype,
                    op,
                    root,
                    comm);
      assert(rc==MPI_SUCCESS);
    }
    if (reduceTgt)
      rc=MPI_Reduce(sbufd,
                    rbufd,
                    count,
                    datatyped,
                    op,
                    root,
                    shadowcomm);
    if (reduceAdj) {
      int dt_idxb = derivedTypeIdx(datatypeb);
      MPI_Aint lbb = (isDerivedType(dt_idxb)?getDTypeData()->lbs[dt_idxb]:0) ;
      void *tmp_bufb =
        (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatypeb,comm);
      tmp_bufb = (void*)((char*)tmp_bufb - lbb);
      if (rank==root)
        (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(rbufb,tmp_bufb,
                                                        count, datatypeb, comm);
      rc=MPI_Bcast(tmp_bufb, count, datatypeb, root, comm) ;
      assert(rc==MPI_SUCCESS) ;
      (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count,datatypeb,comm,sbufb,tmp_bufb, idx) ;
      (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(tmp_bufb,count,datatypeb);
      rc = MPI_SUCCESS ;
    }
    return rc;
  }
}

int FWB_AMPI_Reduce (void* sbuf,
		    void* rbuf,
		    int count,
		    MPI_Datatype datatype,
		    MPI_Op op,
		    int root,
		    MPI_Comm comm) {
  int rc,rank;
  MPI_Comm_rank(comm,&rank);
  int uop_idx = userDefinedOpIdx(op);
  if (isUserDefinedOp(uop_idx)) {
    int comm_size, is_commutative;
    int mask, relrank, source, lroot;
    int dt_idx = derivedTypeIdx(datatype);
    MPI_Status status;
    MPI_Aint lb;
    void *tmp_buf;
    userDefinedOpData* uopd = getUOpData();
    MPI_User_function* uop = uopd->functions[uop_idx];
    if (count == 0) return MPI_SUCCESS;
    MPI_Comm_size(comm,&comm_size);
    if (isDerivedType(dt_idx)) lb = getDTypeData()->lbs[dt_idx];
    else lb = 0;
    is_commutative = uopd->commutes[uop_idx];
    tmp_buf = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatype,comm);
    tmp_buf = (void*)((char*)tmp_buf - lb);
    if (rank != root) {
      rbuf = (*ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp)(count,datatype,comm);
      rbuf = (void*)((char*)rbuf - lb);
    }
    if ((rank != root) || (sbuf != MPI_IN_PLACE)) {
      (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(sbuf, rbuf, count, datatype, comm);
    }
    mask = 0x1;
    if (is_commutative)
      lroot = root;
    else
      lroot = 0;
    relrank = (rank - lroot + comm_size) % comm_size;
    while (mask < comm_size) {
      if ((mask & relrank) == 0) {
	source = (relrank | mask);
	if (source < comm_size) {
	  
	  source = (source + lroot) % comm_size;
	  rc = FW_AMPI_Recv(tmp_buf, count, datatype, source,
			 11, AMPI_FROM_SEND, comm, &status);
	  assert(rc==MPI_SUCCESS);
	  if (is_commutative) {
	    (*uop)(tmp_buf, rbuf, &count, &datatype);
	  }
	  else {
	    (*uop)(rbuf, tmp_buf, &count, &datatype);
	    (*ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp)(sbuf, rbuf, count, datatype, comm);
	  }
	}
      }
      else {
	source = ((relrank & (~mask)) + lroot) % comm_size;
	rc = FW_AMPI_Send(rbuf, count, datatype, source,
			  11, AMPI_TO_RECV, comm);
	assert(rc==MPI_SUCCESS);
	break;
      }
      mask<<=1;
    }
    if (!is_commutative && (root != 0)) {
      if (rank == 0) rc = FW_AMPI_Send(rbuf, count, datatype, root,
				    11, AMPI_TO_RECV, comm);
      else if (rank==root) rc = FW_AMPI_Recv(rbuf, count, datatype, 0,
					  11, AMPI_FROM_SEND, comm, &status);
      assert(rc==MPI_SUCCESS);
    }
    (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(tmp_buf,count,datatype);
    if (rank != root) {
      (*ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp)(rbuf,count,datatype);
      }
    return 0;
  }
  else {
    double* mappedsbuf=NULL;
    double* mappedrbuf=NULL;
    if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      mappedsbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sbuf,&count);
      mappedrbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(rbuf,&count);
    }
    else {
      mappedsbuf=sbuf;
      mappedrbuf=rbuf;
    }
    rc=MPI_Reduce(mappedsbuf,
		  mappedrbuf,
		  count,
		  (*ourADTOOL_AMPI_FPCollection.FW_rawType_fp)(datatype),
		  op,
		  root,
		  comm);
    if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
      (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_REDUCE);
      (*ourADTOOL_AMPI_FPCollection.pushReduceInfo_fp)(sbuf,
						       rbuf,
						       rbuf,
						       rank==root, /* also push contents of rbuf for root */
						       count,
						       datatype,
						       op,
						       root,
						       comm);
    }
    return rc;
  }
}

/** [llh 16/10/2013] This version for Association-By-Name : */
int FWS_AMPI_Reduce(void* sbuf,
                   void* rbuf,
                   int count,
                   MPI_Datatype datatype,
                   MPI_Op op,
                   int root,
                   MPI_Comm comm) {
  return PEDESTRIAN_AMPI_Reduce(sbuf, NULL, NULL,
                         rbuf, NULL, NULL,
                         count,
                         datatype, datatype, datatype, 
                         op, NULL, NULL,
                         1,
                         root,
                         comm) ;
}

int BWB_AMPI_Reduce (void* sbuf,
		    void* rbuf,
		    int count,
		    MPI_Datatype datatype,
		    MPI_Op op,
                    int root,
                    MPI_Comm comm) {
  int rc,rank;
  void *idx=NULL;
  (*ourADTOOL_AMPI_FPCollection.popReduceCountAndType_fp)(&count,&datatype);
  MPI_Datatype mappedtype = (*ourADTOOL_AMPI_FPCollection.BW_rawType_fp)(datatype);
  (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&count,datatype);
  void *prevValBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,datatype,comm);
  void *reduceResultBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,datatype,comm);
  (*ourADTOOL_AMPI_FPCollection.popReduceInfo_fp)(&sbuf,
						  &rbuf,
						  &prevValBuf,
						  &reduceResultBuf,
						  &count,
						  &op,
						  &root,
						  &comm,
						  &idx);
  MPI_Comm_rank(comm,&rank);
  rc=MPI_Bcast(reduceResultBuf,
	       count,
	       mappedtype,
	       root,
	       comm);
  if (rc!=MPI_SUCCESS) MPI_Abort(comm, MPI_ERR_ARG);
  void *tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
  if (rank==root) {
    (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count, mappedtype, comm, tempBuf);
    (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count, mappedtype, comm,
                                                       tempBuf, rbuf, idx);
    (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count, mappedtype, comm, rbuf);
  }
  rc=MPI_Bcast(tempBuf,
	       count,
	       mappedtype,
	       root,
	       comm);
  if (op==MPI_PROD) {
    (*ourADTOOL_AMPI_FPCollection.multiplyAdjoint_fp)(count, mappedtype, comm,
                                                      tempBuf, reduceResultBuf, &idx);
    (*ourADTOOL_AMPI_FPCollection.divideAdjoint_fp)(count, mappedtype, comm,
                                                    tempBuf, prevValBuf, &idx);
  }
  else if (op==MPI_MAX || op==MPI_MIN) {
    void *equalsResultBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
    (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count, mappedtype, comm,
                                                     equalsResultBuf);
    (*ourADTOOL_AMPI_FPCollection.equalAdjoints_fp)(count, mappedtype, comm,
                                                    equalsResultBuf, prevValBuf, reduceResultBuf, &idx);
    void *contributionTotalsBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
    MPI_Allreduce(equalsResultBuf,
		  contributionTotalsBuf,
		  count,
		  mappedtype,
		  MPI_SUM,
		  comm);
    (*ourADTOOL_AMPI_FPCollection.multiplyAdjoint_fp)(count, mappedtype, comm,
                                                      tempBuf, equalsResultBuf, &idx);
    (*ourADTOOL_AMPI_FPCollection.divideAdjoint_fp)(count, mappedtype, comm,
                                                    tempBuf, contributionTotalsBuf, &idx);
    (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(equalsResultBuf);
    (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(contributionTotalsBuf);
  }
  else {}
  (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count, mappedtype, comm,
                                                     sbuf, tempBuf, idx);
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(reduceResultBuf);
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(prevValBuf);
  return rc;
}

/** [llh 16/10/2013] This version for Association-By-Name : */
int BWS_AMPI_Reduce(void* sbuf, void* sbufb,
		   void* rbuf, void* rbufb,
		   int count,
		   MPI_Datatype datatype, MPI_Datatype datatypeb,
		   MPI_Op op, TLM_userFunctionF* uopb,
                   int root,
                   MPI_Comm comm) {
  return PEDESTRIAN_AMPI_Reduce(sbuf, NULL, sbufb,
                         rbuf, NULL, rbufb,
                         count,
                         datatype, datatype, datatypeb,
                         op, NULL, uopb,
                         1,
                         root,
                         comm) ;
}

int TLB_AMPI_Reduce(void* sbuf,
                    void* rbuf,
                    int count,
                    MPI_Datatype datatype,
                    MPI_Op op,
                    int root,
                    MPI_Comm comm){
  int rc=MPI_Reduce(sbuf,
                    rbuf,
                    count,
                    datatype,
                    op,
                    root,
                    comm);
  return rc;
}

/**
 * Tangent diff of \ref AMPI_Reduce. Shadowed (Association-by-Name)
 */
int TLS_AMPI_Reduce(void* sbuf, void* sbufd,
                    void* rbuf, void* rbufd,
                    int count,
                    MPI_Datatype datatype, MPI_Datatype datatyped,
                    MPI_Op op, TLM_userFunctionF* uopd,
                    int root,
                    MPI_Comm comm) {
  return PEDESTRIAN_AMPI_Reduce(sbuf, sbufd, NULL,
                         rbuf, rbufd, NULL,
                         count,
                         datatype, datatyped, datatype,
                         op, uopd, NULL,
                         0,
                         root,
                         comm) ;
}

int FW_AMPI_Allreduce (void* sbuf,
                       void* rbuf,
                       int count,
                       MPI_Datatype datatype,
                       MPI_Op op,
                       MPI_Comm comm) {
  int rc,rank;
  MPI_Comm_rank(comm,&rank);
  double* mappedsbuf=NULL;
  double* mappedrbuf=NULL;
  if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
    mappedsbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(sbuf,&count);
    mappedrbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(rbuf,&count);
  }
  else {
    mappedsbuf=sbuf;
    mappedrbuf=rbuf;
  }
  rc=MPI_Allreduce(mappedsbuf,
                   mappedrbuf,
                   count,
                   (*ourADTOOL_AMPI_FPCollection.FW_rawType_fp)(datatype),
                   op,
                   comm);
  if (rc==MPI_SUCCESS && (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE) {
    (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_ALLREDUCE);
    (*ourADTOOL_AMPI_FPCollection.pushReduceInfo_fp)(sbuf,
						     rbuf,
						     rbuf,
						     1,
						     count,
						     datatype,
						     op,
						     0,
						     comm);
  }
  return rc;
}

int BW_AMPI_Allreduce (void* sbuf,
                       void* rbuf,
                       int count,
                       MPI_Datatype datatype,
                       MPI_Op op,
                       MPI_Comm comm) {
  int rc=0,rank, rootPlaceHolder;
  void *idx=NULL;
  (*ourADTOOL_AMPI_FPCollection.popReduceCountAndType_fp)(&count,&datatype);
  MPI_Datatype mappedtype = (*ourADTOOL_AMPI_FPCollection.BW_rawType_fp)(datatype);
  (*ourADTOOL_AMPI_FPCollection.getAdjointCount_fp)(&count,datatype);
  void *prevValBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
  void *reduceResultBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
  (*ourADTOOL_AMPI_FPCollection.popReduceInfo_fp)(&sbuf,
						  &rbuf,
						  &prevValBuf,
						  &reduceResultBuf,
						  &count,
						  &op,
						  &rootPlaceHolder,
						  &comm,
						  &idx);
  MPI_Comm_rank(comm,&rank);
  void *tempBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
  MPI_Allreduce(rbuf,
                tempBuf,
                count,
                mappedtype,
                MPI_SUM,
                comm);
  if (op==MPI_SUM) {
     ; /* nothing extra to be done here */
  }
  else if (op==MPI_PROD) {
    (*ourADTOOL_AMPI_FPCollection.multiplyAdjoint_fp)(count, mappedtype, comm,
                                                      tempBuf, reduceResultBuf, &idx);
    (*ourADTOOL_AMPI_FPCollection.divideAdjoint_fp)(count, mappedtype, comm,
                                                    tempBuf, prevValBuf, &idx);
  }
  else if (op==MPI_MAX || op==MPI_MIN) {
    void *equalsResultBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp)(count,mappedtype,comm);
    (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count, mappedtype, comm,
                                                     equalsResultBuf);
    (*ourADTOOL_AMPI_FPCollection.equalAdjoints_fp)(count, mappedtype, comm,
                                                   equalsResultBuf, prevValBuf, reduceResultBuf, &idx);
    void *contributionTotalsBuf = (*ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp) (count,mappedtype,comm);
    MPI_Allreduce(equalsResultBuf,
                  contributionTotalsBuf,
                  count,
                  mappedtype,
                  MPI_SUM,
                  comm);
    (*ourADTOOL_AMPI_FPCollection.multiplyAdjoint_fp)(count, mappedtype, comm,
                                                      tempBuf, equalsResultBuf, &idx);
    (*ourADTOOL_AMPI_FPCollection.divideAdjoint_fp)(count, mappedtype, comm,
                                                    tempBuf, contributionTotalsBuf, &idx);
    (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(equalsResultBuf);
    (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(contributionTotalsBuf);
  }
  else {
    assert(0); /* unimplemented */
  }
  (*ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp)(count, mappedtype, comm,
                                                     sbuf, tempBuf, idx);
  (*ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp)(count, mappedtype, comm, rbuf);
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(tempBuf);
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(reduceResultBuf);
  (*ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp)(prevValBuf);
  return rc;
}

int TLM_AMPI_Allreduce(void* sbuf,
                       void* rbuf,
                       int count,
                       MPI_Datatype datatype,
                       MPI_Op op,
                       MPI_Comm comm) {
  int rc=0;
  assert(0);
  return rc;
}

derivedTypeData* getDTypeData() {
  static derivedTypeData* dat = NULL;
  if (dat==NULL) {
    derivedTypeData* newdat = (derivedTypeData*)malloc(sizeof(derivedTypeData));
    newdat->size = 0;
    newdat->preAlloc = 0;
    newdat->num_actives = NULL;
    newdat->first_active_blocks = NULL;
    newdat->last_active_blocks = NULL;
    newdat->last_active_block_lengths = NULL;
    newdat->derived_types = NULL;
    newdat->counts = NULL;
    newdat->arrays_of_blocklengths = NULL;
    newdat->arrays_of_displacements = NULL;
    newdat->arrays_of_types = NULL;
    newdat->lbs = NULL;
    newdat->extents = NULL;
    newdat->packed_types = NULL;
    newdat->arrays_of_p_blocklengths = NULL;
    newdat->arrays_of_p_displacements = NULL;
    newdat->arrays_of_p_types = NULL;
    newdat->p_extents = NULL;
    dat = newdat;
  }
  return dat;
}

void releaseDTypeData() {
  int i;
  derivedTypeData* dat = getDTypeData();
  for (i=0;i<dat->size;i++) {
    free(dat->arrays_of_blocklengths[i]);
    free(dat->arrays_of_displacements[i]);
    free(dat->arrays_of_types[i]);
    free(dat->arrays_of_p_blocklengths[i]);
    free(dat->arrays_of_p_displacements[i]);
    free(dat->arrays_of_p_types[i]);
    if (dat->packed_types[i]!=MPI_DATATYPE_NULL) MPI_Type_free(dat->packed_types+i);
  }
  free(dat->num_actives);
  free(dat->first_active_blocks);
  free(dat->last_active_blocks);
  free(dat->last_active_block_lengths);
  free(dat->derived_types);
  free(dat->counts);
  free(dat->arrays_of_blocklengths);
  free(dat->arrays_of_displacements);
  free(dat->arrays_of_types);
  free(dat->lbs);
  free(dat->extents);
  free(dat->packed_types);
  free(dat->arrays_of_p_blocklengths);
  free(dat->arrays_of_p_displacements);
  free(dat->arrays_of_p_types);
  free(dat->p_extents);
  free(dat);
}

void releaseUOpData() {
  userDefinedOpData* dat = getUOpData();
  free(dat->ops);
  free(dat->functions);
  free(dat->commutes);
  free(dat);
}

void addDTypeData(derivedTypeData* dat,
		  int count,
		  int array_of_blocklengths[],
		  MPI_Aint array_of_displacements[],
		  MPI_Datatype array_of_types[],
		  MPI_Aint lb,
		  MPI_Aint extent,
		  int array_of_p_blocklengths[],
		  MPI_Aint array_of_p_displacements[],
		  MPI_Datatype array_of_p_types[],
		  MPI_Aint p_extent,
		  MPI_Datatype* newtype,
		  MPI_Datatype* packed_type) {
  assert(dat);
  int i, dt_idx;
  int num_actives=0, fst_ablk_set=0;
  MPI_Aint fst_active_blk=0, lst_active_blk=0, lst_active_blk_len=0;
  for (i=0;i<count;i++) {
    if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(array_of_types[i])==AMPI_ACTIVE) {
      num_actives += array_of_blocklengths[i];
      if (!fst_ablk_set) {
	fst_active_blk = array_of_displacements[i];
	fst_ablk_set = 1;
      }
      lst_active_blk = array_of_displacements[i];
      lst_active_blk_len = array_of_blocklengths[i];
      continue;
    }
    dt_idx = derivedTypeIdx(array_of_types[i]);
    if (isDerivedType(dt_idx)) {
      num_actives += dat->num_actives[dt_idx]*array_of_blocklengths[i];
      if (!fst_ablk_set) {
	fst_active_blk = array_of_displacements[i] + dat->first_active_blocks[dt_idx];
	fst_ablk_set = 1;
      }
      lst_active_blk = array_of_displacements[i] + (array_of_blocklengths[i]-1)*dat->extents[dt_idx] + dat->last_active_blocks[dt_idx];
      lst_active_blk_len = dat->last_active_block_lengths[dt_idx];
    }
  }
  if (dat->preAlloc == dat->size) {
    dat->preAlloc += 16;
    dat->num_actives = realloc(dat->num_actives, (dat->preAlloc)*sizeof(int));
    dat->first_active_blocks = realloc(dat->first_active_blocks, (dat->preAlloc)*sizeof(MPI_Aint));
    dat->last_active_blocks = realloc(dat->last_active_blocks, (dat->preAlloc)*sizeof(MPI_Aint));
    dat->last_active_block_lengths = realloc(dat->last_active_block_lengths, (dat->preAlloc)*sizeof(int));
    dat->derived_types = realloc(dat->derived_types,
				 (dat->preAlloc)*sizeof(MPI_Datatype));
    dat->counts = realloc(dat->counts, (dat->preAlloc)*sizeof(int));
    dat->arrays_of_blocklengths = realloc(dat->arrays_of_blocklengths,
					  (dat->preAlloc)*sizeof(int*));
    dat->arrays_of_displacements = realloc(dat->arrays_of_displacements,
					   (dat->preAlloc)*sizeof(MPI_Aint*));
    dat->arrays_of_types = realloc(dat->arrays_of_types,
				   (dat->preAlloc)*sizeof(MPI_Datatype*));
    dat->lbs = realloc(dat->lbs, (dat->preAlloc)*sizeof(MPI_Aint));
    dat->extents = realloc(dat->extents, (dat->preAlloc)*sizeof(MPI_Aint));
    dat->packed_types = realloc(dat->packed_types,
				(dat->preAlloc)*sizeof(MPI_Datatype));
    dat->arrays_of_p_blocklengths = realloc(dat->arrays_of_p_blocklengths,
					    (dat->preAlloc)*sizeof(int*));
    dat->arrays_of_p_displacements = realloc(dat->arrays_of_p_displacements,
					     (dat->preAlloc)*sizeof(MPI_Aint*));
    dat->arrays_of_p_types = realloc(dat->arrays_of_p_types,
				     (dat->preAlloc)*sizeof(MPI_Datatype*));
    dat->p_extents = realloc(dat->p_extents, (dat->preAlloc)*sizeof(MPI_Aint));
  }
  dat->num_actives[dat->size] = num_actives;
  dat->first_active_blocks[dat->size] = fst_active_blk;
  dat->last_active_blocks[dat->size] = lst_active_blk;
  dat->last_active_block_lengths[dat->size] = lst_active_blk_len;
  dat->derived_types[dat->size] = *newtype;
  dat->counts[dat->size] = count;
  dat->arrays_of_blocklengths[dat->size] = malloc(count*sizeof(int));
  memcpy(dat->arrays_of_blocklengths[dat->size], array_of_blocklengths, count*sizeof(int));
  dat->arrays_of_displacements[dat->size] = malloc(count*sizeof(MPI_Aint));
  memcpy(dat->arrays_of_displacements[dat->size], array_of_displacements, count*sizeof(MPI_Aint));
  dat->arrays_of_types[dat->size] = malloc(count*sizeof(MPI_Datatype));
  memcpy(dat->arrays_of_types[dat->size], array_of_types, count*sizeof(MPI_Datatype));
  dat->lbs[dat->size] = lb;
  dat->extents[dat->size] = extent;
  dat->packed_types[dat->size] = *packed_type;
  dat->arrays_of_p_blocklengths[dat->size] = malloc(count*sizeof(int));
  memcpy(dat->arrays_of_p_blocklengths[dat->size], array_of_p_blocklengths, count*sizeof(int));
  dat->arrays_of_p_displacements[dat->size] = malloc(count*sizeof(MPI_Aint));
  memcpy(dat->arrays_of_p_displacements[dat->size], array_of_p_displacements, count*sizeof(MPI_Aint));
  dat->arrays_of_p_types[dat->size] = malloc(count*sizeof(MPI_Datatype));
  memcpy(dat->arrays_of_p_types[dat->size], array_of_p_types, count*sizeof(MPI_Datatype));
  dat->p_extents[dat->size] = p_extent;
  dat->size += 1;
}

int derivedTypeIdx(MPI_Datatype datatype) {
  int i;
  derivedTypeData* dtdata = getDTypeData();
  for (i=0;i<dtdata->size;i++) {
    if (dtdata->derived_types[i]==datatype) return i;
  }
  return -1;
}

int isDerivedType(int dt_idx) {
  return dt_idx!=-1;
}

userDefinedOpData* getUOpData() {
  static userDefinedOpData* dat = NULL;
  if (dat==NULL) {
    userDefinedOpData* newdat = (userDefinedOpData*)malloc(sizeof(userDefinedOpData));
    newdat->size = 0;
    newdat->preAlloc = 0;
    newdat->ops = NULL;
    newdat->functions = NULL;
    newdat->commutes = NULL;
    dat = newdat;
  }
  return dat;
}

void addUOpData(userDefinedOpData* dat,
		MPI_Op* op,
		MPI_User_function* function,
		int commute) {
  assert(dat);
  if (dat->preAlloc == dat->size) {
    dat->preAlloc += 16;
    dat->ops = realloc(dat->ops,(dat->preAlloc)*sizeof(MPI_Op));
    dat->functions = realloc(dat->functions,(dat->preAlloc)*sizeof(MPI_User_function*));
    dat->commutes = realloc(dat->commutes,(dat->preAlloc)*sizeof(int));
  }
  dat->ops[dat->size] = *op;
  dat->functions[dat->size] = function;
  dat->commutes[dat->size] = commute;
  dat->size += 1;
}

int userDefinedOpIdx(MPI_Op op) {
  int i;
  userDefinedOpData* uopdata = getUOpData();
  for (i=0;i<uopdata->size;i++) {
    if (uopdata->ops[i]==op) return i;
  }
  return -1;
}

int isUserDefinedOp(int uop_idx) {
  return uop_idx!=-1;
}

/* One-sided AMPI */
int FW_AMPI_Win_create( void *base,
    MPI_Aint size,
    int disp_unit,
    MPI_Info info,
    MPI_Comm comm,
    AMPI_Win *win
    )
{
  int rc=0;
  win->req_stack=(AMPI_Win_stack *) malloc(sizeof(AMPI_Win_stack));
  AMPI_WIN_STACK_stack_init(win->req_stack);
  win->map=malloc(sizeof(void*));
  *win->map=(ourADTOOL_AMPI_FPCollection.createWinMap_fp)(base,size);
  win->plainWindow=(MPI_Win**) malloc(sizeof(MPI_Win*));
  *win->plainWindow= (MPI_Win*) malloc(sizeof(MPI_Win));
  win->base=base;
  win->num_reqs=0;
  win->idx=NULL;
  win->size=(ourADTOOL_AMPI_FPCollection.getWinSize_fp)(size);
  win->disp=disp_unit;
  win->comm=comm;
  (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_WIN_CREATE);
  rc=MPI_Win_create(*win->map, win->size, win->disp, info, win->comm, *win->plainWindow);
  (*ourADTOOL_AMPI_FPCollection.push_AMPI_Win_fp)(win);
  /*BK_AMPI_put_AMPI_Win(win);*/
  return rc;
}

int BW_AMPI_Win_create( void *base,
    MPI_Aint size,
    int disp_unit,
    MPI_Info info,
    MPI_Comm comm,
    AMPI_Win *win
    ) {
  int rc=0;
  (*ourADTOOL_AMPI_FPCollection.pop_AMPI_Win_fp)(win);
  AMPI_WIN_STACK_destroy(win->req_stack);
  free(win->req_stack);
  free(*win->map);
  free(win->map);
  rc=MPI_Win_free(*win->plainWindow);
  free(*win->plainWindow);
  free(win->plainWindow);
  return rc;
}

int FW_AMPI_Win_free( AMPI_Win *win ) {
  free(*win->map);
  (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_WIN_FREE);
  (*ourADTOOL_AMPI_FPCollection.push_AMPI_Win_fp)(win);
  return MPI_Win_free(*win->plainWindow);
}

int BW_AMPI_Win_free( AMPI_Win *win ) {
  int rc=0;
  AMPI_Win ampiWin;
  (*ourADTOOL_AMPI_FPCollection.pop_AMPI_Win_fp)(&ampiWin);
  *ampiWin.map=malloc(sizeof(ampiWin.size));
  /**ampiWin.plainWindow=malloc(sizeof(MPI_Win));*/
  rc=MPI_Win_create(*ampiWin.map, ampiWin.size, ampiWin.disp, MPI_INFO_NULL, ampiWin.comm, *ampiWin.plainWindow);
  double *map_=(double*) *ampiWin.map;
  if(ampiWin.size!=0)
    map_[0]=0;
  return rc;
}

int FW_AMPI_Get( void *origin_addr,
    int origin_count,
    MPI_Datatype origin_datatype,
    int target_rank,
    MPI_Aint target_disp,
    int target_count,
    MPI_Datatype target_datatype,
    AMPI_Win win
    ) 
{  
  int rc=MPI_SUCCESS;
  double* mappedbuf=NULL;
  if((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(origin_datatype)==AMPI_ACTIVE) {
    mappedbuf=(*ourADTOOL_AMPI_FPCollection.rawData_fp)(origin_addr,&origin_count);
  }
  else {
    mappedbuf=origin_addr;
  }
  rc=MPI_Get( mappedbuf,
      /*rc=MPI_Get( origin_addr,*/
      origin_count,
      origin_datatype,
      target_rank,
      target_disp,
      target_count,
      target_datatype,
      **win.plainWindow
      );
  AMPI_WinRequest winRequest;
  /* fill in the other info */
  winRequest.origin_addr=origin_addr;
  winRequest.origin_count=origin_count;
  winRequest.origin_datatype=origin_datatype;
  winRequest.target_rank=target_rank;
  winRequest.target_disp=target_disp;
  winRequest.target_count=target_count;
  winRequest.target_datatype=target_datatype;
  (*ourADTOOL_AMPI_FPCollection.mapWinBufForAdjoint_fp)(&winRequest,origin_addr);
  if ((*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(origin_datatype)==AMPI_ACTIVE) {
    (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_GET);
    AMPI_WIN_STACK_push(win.req_stack,winRequest);
  }
  return rc;
}

int BW_AMPI_Get( void *origin_addr,
    int origin_count,
    MPI_Datatype origin_datatype,
    int target_rank,
    MPI_Aint target_disp,
    int target_count,
    MPI_Datatype target_datatype,
    AMPI_Win win
    ) {
  return MPI_SUCCESS;
}

int FW_AMPI_Put( void *origin_addr,
    int origin_count,
    MPI_Datatype origin_datatype,
    int target_rank,
    MPI_Aint target_disp,
    int target_count,
    MPI_Datatype target_datatype,
    AMPI_Win win
    ) {
  return MPI_SUCCESS;
}

int BW_AMPI_Put( void *origin_addr,
    int origin_count,
    MPI_Datatype origin_datatype,
    int target_rank,
    MPI_Aint target_disp,
    int target_count,
    MPI_Datatype target_datatype,
    AMPI_Win win
    ) {
  return MPI_SUCCESS;
}

int FW_AMPI_Win_fence( int assert,
    AMPI_Win win
    )
{
  AMPI_WinRequest winRequest;
  int rc=MPI_SUCCESS;
  int i=0;
  int num_reqs=0;
  printf("FW win ptr: %p\n", *win.plainWindow);
  MPI_Win tmp=**win.plainWindow;
  /*Sync window*/
  rc=MPI_Win_fence( assert, tmp);
  (ourADTOOL_AMPI_FPCollection.writeWinData_fp)(*win.map,win.base,win.size);

  num_reqs=win.req_stack->num_reqs;
  (*ourADTOOL_AMPI_FPCollection.push_CallCode_fp)(AMPI_WIN_FENCE);
  for(i=num_reqs; i>0 ; i=i-1) {
    winRequest=AMPI_WIN_STACK_pop(win.req_stack);
    (*ourADTOOL_AMPI_FPCollection.writeData_fp)(winRequest.origin_addr,&winRequest.origin_count);
    (*ourADTOOL_AMPI_FPCollection.push_AMPI_WinRequest_fp)(&winRequest);
  }
  win.num_reqs=num_reqs;
  printf("FW num_reqs: %d\n", win.num_reqs);
  win.req_stack->num_reqs=0;
  (*ourADTOOL_AMPI_FPCollection.push_AMPI_Win_fp)(&win);
  rc=MPI_Win_fence( assert, **win.plainWindow );
  return rc;
}

int BW_AMPI_Win_fence( int assert,
    AMPI_Win win
    )
{
  AMPI_WinRequest winRequest;
  int rc=MPI_SUCCESS;
  int i=0;
  int num_reqs=0;
  assert=0;

  /* We pop the window from the tape. Here we save the MPI_Win for the adjoints */

  (*ourADTOOL_AMPI_FPCollection.pop_AMPI_Win_fp)(&win);
  printf("BW win ptr: %p\n", *win.plainWindow);

  /* First part is copying the adjoints. With booking we look up how many 1sided
   * adjoint comms took place*/

  rc=MPI_Win_fence( assert, **win.plainWindow );
  /*AMPI_Win bk_win;*/
  /*BK_AMPI_read_AMPI_Win(*win.plainWindow,&bk_win);*/
  /*printf("BW bk_num_reqs: %ld\n", bk_win.req_stack->num_reqs);*/
  /*printf("BW bk_win.size: %ld\n", bk_win.size);*/


  /*(ourADTOOL_AMPI_FPCollection.writeWinData_fp)(win.map,win.base,win.size);*/
  /*double *tmp=(double *) win.map;*/
  double *tmp=(double *) *win.map;

  /* if window size is nonzero we sync the incoming adjoints in the window map
   * and set the map to zero again */

  if(win.size!=0) {
    printf("BW Fence map: %f\n", tmp[0]);
    (*ourADTOOL_AMPI_FPCollection.syncAdjointWin_fp)(&win);
  }
  /*num_reqs=bk_win.req_stack->num_reqs;*/

  /* placeholder for adjoints that are receveived through get. have to copy them
   * back here */

  /*for(i=num_reqs; i>0 ; i=i-1) {*/
  /*bk_winRequest=AMPI_WIN_STACK_pop(bk_win.req_stack);*/
  /*}*/

  /* We dispatch the next adjoint communications. These are popped from the
   * tape.*/

  rc=MPI_Win_fence( assert, **win.plainWindow );
  num_reqs=win.num_reqs;
  printf("BW num_reqs: %d\n", num_reqs);
  for(i=num_reqs; i>0 ; i=i-1) {
    (*ourADTOOL_AMPI_FPCollection.pop_AMPI_WinRequest_fp)(&winRequest);
    (*ourADTOOL_AMPI_FPCollection.setWinAdjointCountAndTempBuf_fp)(&winRequest);
    double *tmp=(double *) ((*ourADTOOL_AMPI_FPCollection.rawAdjointData_fp)(winRequest.adjointTempBuf));
    printf("BW Put adj: %f\n", tmp[0]);
    rc=MPI_Put( (*ourADTOOL_AMPI_FPCollection.rawAdjointData_fp)(winRequest.adjointTempBuf),
	winRequest.origin_count,
	winRequest.origin_datatype,
	winRequest.target_rank,
	winRequest.target_disp,
	winRequest.target_count,
	winRequest.target_datatype,
	**win.plainWindow
	);

    /*And we save the adjoint comms in our window that is in the bk system*/

    /*AMPI_WIN_STACK_push(bk_win.req_stack,*winRequest);*/
  }
  return rc;
}
