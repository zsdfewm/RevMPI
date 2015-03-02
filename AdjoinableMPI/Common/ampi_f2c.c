/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/

#include <stdlib.h>
#include <mpi.h>

#include "ampi/userIF/activity.h"
#include "ampi/userIF/pairedWith.h"
#include "ampi/userIF/request.h"
#include "ampi/userIF/nt.h"
#include "ampi/adTool/support.h"
#include "ampi/libCommon/modified.h"
#include "ampi/ampi.h"

AMPI_PairedWith pairedWithTable[] =
  {AMPI_TO_RECV, AMPI_FROM_SEND, AMPI_TO_IRECV_WAIT, AMPI_TO_IRECV_WAITALL,
   AMPI_FROM_ISEND_WAIT, AMPI_FROM_ISEND_WAITALL, AMPI_FROM_BSEND, AMPI_FROM_RSEND} ;

MPI_Fint ampi_adouble_precision_;
MPI_Fint ampi_areal_;

void ampi_init_nt_(int* err_code) {
  *err_code = AMPI_Init_NT(0, 0);
}

void ampi_finalize_nt_(int* err_code) {
  *err_code = AMPI_Finalize_NT();}

void ampi_comm_rank_(MPI_Fint *commF, int *rank, int* err_code) {
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = MPI_Comm_rank(commC, rank);
}

void adtool_ampi_turn_(double *v, double *vb) {
  ADTOOL_AMPI_Turn(v, vb) ;
}

void fw_ampi_recv_(void* buf,
                   int *count,
                   MPI_Fint *datatypeF,
                   int *src,
                   int *tag,
                   int *pairedWithF,
                   int *commF,
                   int *status,
                   int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = FW_AMPI_Recv(buf, *count, datatype,
                           *src, *tag, pairedWith, commC,
                           (MPI_Status*)status);
}

void bw_ampi_recv_(void* buf,
                   int *count,
                   MPI_Fint *datatypeF,
                   int* src,
                   int* tag,
                   int* pairedWithF,
                   int* commF,
                   int* status,
                   int* err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = BW_AMPI_Recv(buf, *count, datatype,
                           *src, *tag, pairedWith, commC,
                           (MPI_Status*)status);
}

void tls_ampi_recv_(void* buf, void* shadowbuf,
                    int *count,
                    MPI_Fint *datatypeF, MPI_Fint *shadowdatatypeF,
                    int* src,
                    int* tag,
                    int* pairedWithF,
                    int* commF,
                    int* status,
                    int* err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Datatype shadowdatatype = MPI_Type_f2c(*shadowdatatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = TLS_AMPI_Recv(buf, shadowbuf, *count, datatype, shadowdatatype,
                           *src, *tag, pairedWith, commC,
                           (MPI_Status*)status);
}

void fw_ampi_irecv_(void* buf,
                    int *count,
                    MPI_Fint *datatypeF,
                    int *source,
                    int *tag,
                    int *pairedWithF,
                    int *commF,
                    int *request,
                    int *err_code){
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = FW_AMPI_Irecv(buf, *count, datatype,
                            *source, *tag, pairedWith, commC,
                            (MPI_Request*)request);
}

void bw_ampi_irecv_(void* buf,
                    int *count,
                    MPI_Fint *datatypeF,
                    int *source,
                    int *tag,
                    int *pairedWithF,
                    int *commF,
                    int *request,
                    int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = BW_AMPI_Irecv(buf, *count, datatype,
                            *source, *tag, pairedWith, commC,
                            (MPI_Request*)request);
}

void tls_ampi_irecv_(void* buf, void* shadowbuf,
                     int *count,
                     MPI_Fint *datatypeF, MPI_Fint *shadowdatatypeF,
                     int *source,
                     int *tag,
                     int *pairedWithF,
                     int *commF,
                     int *request,
                     int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Datatype shadowdatatype = MPI_Type_f2c(*shadowdatatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = TLS_AMPI_Irecv(buf, shadowbuf, *count, datatype, shadowdatatype,
                            *source, *tag, pairedWith, commC,
                            (MPI_Request*)request);
}

void fw_ampi_send_(void* buf, 
                   int *count, 
                   MPI_Fint *datatypeF, 
                   int *dest, 
                   int *tag,
                   int *pairedWithF,
                   int *commF,
                   int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = FW_AMPI_Send (buf, *count, datatype,
                            *dest, *tag, pairedWith, commC);
}

void bw_ampi_send_(void* buf,
                   int *count,
                   MPI_Fint *datatypeF,
                   int *dest, 
                   int *tag,
                   int *pairedWithF,
                   int *commF,
                   int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = BW_AMPI_Send (buf, *count, datatype,
                            *dest, *tag, pairedWith, commC);
}

void tls_ampi_send_(void* buf, void* shadowbuf,
                    int *count,
                    MPI_Fint *datatypeF, MPI_Fint *shadowdatatypeF,
                    int *dest, 
                    int *tag,
                    int *pairedWithF,
                    int *commF,
                    int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Datatype shadowdatatype = MPI_Type_f2c(*shadowdatatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = TLS_AMPI_Send (buf, shadowbuf, *count, datatype, shadowdatatype,
                             *dest, *tag, pairedWith, commC);
}

void fw_ampi_isend_(void* buf,
                    int *count,
                    MPI_Fint *datatypeF,
                    int *dest,
                    int *tag,
                    int *pairedWithF,
                    int *commF,
                    int *request,
                    int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = FW_AMPI_Isend(buf, *count, datatype,
                            *dest, *tag, pairedWith, commC,
                            (MPI_Request*)request);
}

void bw_ampi_isend_(void* buf,
                    int *count,
                    MPI_Fint *datatypeF,
                    int *dest,
                    int *tag,
                    int *pairedWithF,
                    int *commF,
                    int *request,
                    int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = BW_AMPI_Isend(buf, *count, datatype,
                            *dest, *tag, pairedWith, commC,
                            (MPI_Request*)request);
}

void tls_ampi_isend_(void* buf, void* shadowbuf,
                     int *count,
                     MPI_Fint *datatypeF, MPI_Fint *shadowdatatypeF,
                     int *dest,
                     int *tag,
                     int *pairedWithF,
                     int *commF,
                     int *request,
                     int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Datatype shadowdatatype = MPI_Type_f2c(*shadowdatatypeF) ;
  AMPI_PairedWith pairedWith = pairedWithTable[*pairedWithF] ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = TLS_AMPI_Isend(buf, shadowbuf, *count, datatype, shadowdatatype,
                            *dest, *tag, pairedWith, commC,
                            (MPI_Request*)request);
}

void fw_ampi_wait_(int *request, int *status, int* err_code) {
  *err_code = FW_AMPI_Wait((MPI_Request*)request,
                           (MPI_Status*)status);
}

void bw_ampi_wait_(int *request, int *status, int* err_code) {
  *err_code = BW_AMPI_Wait((MPI_Request*)request,
                           (MPI_Status*)status);
}

void tls_ampi_wait_(int *request, int *status, int* err_code) {
  *err_code = TLS_AMPI_Wait((MPI_Request*)request,
                            (MPI_Status*)status);
}

void fw_ampi_barrier_(int *commF, int* err_code) {
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = FW_AMPI_Barrier(commC) ;
}

void bw_ampi_barrier_(int *commF, int* err_code) {
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = BW_AMPI_Barrier(commC) ;
}

void tls_ampi_barrier_(int *commF, int* err_code) {
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = TLS_AMPI_Barrier(commC) ;
}

void fws_ampi_reduce_(void* sbuf, void* rbuf,
                      int *count,
                      MPI_Fint *datatypeF,
                      MPI_Fint *opF,
                      int *root,
                      int *commF,
                      int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Op op = MPI_Op_f2c(*opF) ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = FWS_AMPI_Reduce(sbuf, rbuf, *count, datatype,
                              op, *root, commC) ;
}

void bws_ampi_reduce_(void* sbuf, void* sbufb,
                      void* rbuf, void* rbufb,
                      int *count,
                      MPI_Fint *datatypeF, MPI_Fint *datatypebF,
                      MPI_Fint *opF, void* uopbF,
                      int *root,
                      int *commF,
                      int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Datatype datatypeb = MPI_Type_f2c(*datatypebF) ;
  MPI_Op op = MPI_Op_f2c(*opF) ;
  TLM_userFunctionF* uopb = 0 /*???(uopbF)*/ ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = BWS_AMPI_Reduce(sbuf, sbufb,
                              rbuf, rbufb,
                              *count,
                              datatype, datatypeb,
                              op, uopb,
                              *root, commC) ;
}

void tls_ampi_reduce_(void* sbuf, void* shadowsbuf,
                      void* rbuf, void* shadowrbuf,
                      int *count,
                      MPI_Fint *datatypeF, MPI_Fint *shadowdatatypeF,
                      MPI_Fint *opF, void* uopdF,
                      int *root,
                      int *commF,
                      int *err_code) {
  MPI_Datatype datatype = MPI_Type_f2c(*datatypeF) ;
  MPI_Datatype shadowdatatype = MPI_Type_f2c(*shadowdatatypeF) ;
  MPI_Op op = MPI_Op_f2c(*opF) ;
  TLM_userFunctionF* uopd = 0 /*???(uopdF)*/ ;
  MPI_Comm commC = MPI_Comm_f2c( *commF ) ;
  *err_code = TLS_AMPI_Reduce(sbuf, shadowsbuf,
                              rbuf, shadowrbuf,
                              *count,
                              datatype, shadowdatatype,
                              op, uopd,
                              *root, commC) ;
}
