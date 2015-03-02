/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <mpi.h>
#include "ampi/libCommon/st.h"
#include "ampi/bookkeeping/support.h"
#include "ampi/adTool/support.h"

int FW_AMPI_Wait_ST(AMPI_Request *request,
		    void*  buf,
		    MPI_Status *status) { 
  MPI_Request *plainRequest;
  struct AMPI_Request_S *ampiRequest;
#ifdef AMPI_FORTRANCOMPATIBLE
  struct AMPI_Request_S ampiRequestInst;
  ampiRequest=&ampiRequestInst;
  plainRequest=request;
  BK_AMPI_get_AMPI_Request(plainRequest,ampiRequest,0);
#else 
  plainRequest=&(request->plainRequest);
  ampiRequest=request;
#endif 
  /* push request  */
  ADTOOL_AMPI_push_AMPI_Request(ampiRequest);
  return MPI_Wait(plainRequest,
		  status);
}

int BW_AMPI_Wait_ST(AMPI_Request *request,
		    void*  buf,
		    MPI_Status *status) {
  int rc; 
  struct AMPI_Request_S ampiRequest;
  /* pop request  */
  ADTOOL_AMPI_pop_AMPI_Request(&ampiRequest);
  ADTOOL_AMPI_setBufForAdjoint(&ampiRequest,buf);   
  switch(ampiRequest.origin) { 
  case AMPI_SEND_ORIGIN: { 
    ADTOOL_AMPI_setAdjointCountAndTempBuf(&ampiRequest);   
    rc=MPI_Irecv(ampiRequest.adjointTempBuf,
	      ampiRequest.adjointCount,
	      ampiRequest.datatype,
	      ampiRequest.endPoint,
	      ampiRequest.tag,
	      ampiRequest.comm,
	      &(ampiRequest.plainRequest));
    break;
  }
  case AMPI_RECV_ORIGIN: { 
    ADTOOL_AMPI_setAdjointCount(&ampiRequest);
    rc=MPI_Isend(ampiRequest.buf,
	      ampiRequest.adjointCount,
	      ampiRequest.datatype,
	      ampiRequest.endPoint,
	      ampiRequest.tag,
	      ampiRequest.comm,
	      &(ampiRequest.plainRequest));
    break;
  }
  default:  
    rc=MPI_Abort(ampiRequest.comm, MPI_ERR_TYPE);
    break;
  }
#ifdef AMPI_FORTRANCOMPATIBLE
  *request=ampiRequest.plainRequest;
  BK_AMPI_put_AMPI_Request(&ampiRequest);
#endif
  return rc;
}
