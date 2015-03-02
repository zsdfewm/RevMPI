#include <cassert>

#include "ampi/ampi.h"
#include "ampi/adTool/support.h"
#include "ampi/tape/support.h"
#include "ampi/libCommon/modified.h"
#include "uthash.h"

#include "dco.hpp"

#define INT64 int

using namespace dco::a1s;

typedef struct {
    void *key;
    double *ptr_buf;
    UT_hash_handle hh;
} AMPI_ht_el;

class external_function_ampi_data : public tape::external_function_base_data {
    public:
	int opcode;
	external_function_ampi_data() : opcode(0) { }
};

AMPI_ht_el *AMPI_ht=NULL;


inline void ampi_get_val(void *buf, int *i, double *x) {
    *x=static_cast<type*>(buf)[*i]._value();
}
inline void ampi_set_val(void* buf, int *i, double *v) {
    type &dummy= static_cast<type*>(buf)[*i];
    *const_cast<double*>(&(dummy._value())) = *v;
}

inline void ampi_get_idx(void *buf, int *i, INT64 *idx) {
    type &var = static_cast<type*>(buf)[*i];
    if(!var._data()._is_registered()) {
	*idx=0;
    }
    else {
	*idx = var._data().tape_index();
    }

}


inline void ampi_get_adj(INT64 *idx, double *x) {
    if(*idx!=0) *x = dco::a1s::global_tape->_adjoint(*idx);
}
inline void ampi_set_adj(INT64 *idx, double *x) {
    if(*idx!=0) const_cast<double&>(dco::a1s::global_tape->_adjoint(*idx)) += *x;
}

void ampi_tape_wrapper(tape &caller, const tape::interpretation_settings &settings, dco::a1s::tape::external_function_base_data *userdata) {
    external_function_ampi_data *ext_function_arguments=static_cast<external_function_ampi_data*> (userdata);
    int rc=0;
    int opcode=ext_function_arguments->opcode;
    void *buf;
    int count;
    MPI_Datatype datatype;
    int src;
    int dest;
    int tag;
    AMPI_PairedWith pairedWith;
    MPI_Comm comm;
    AMPI_Request request;
    MPI_Status status;
    //One-sided stuff
    AMPI_Win win;
    void *base;
    MPI_Aint size;
    int disp_unit;
    MPI_Info info;
    int assert=0;
    void *origin_addr;
    void *target_addr;
    int origin_count;
    int target_count;
    MPI_Aint target_disp;
    int target_rank;
    MPI_Datatype origin_datatype;
    MPI_Datatype target_datatype;
    switch(opcode) {
	case AMPI_WAIT:
	    rc=BW_AMPI_Wait(&request,&status);
	    break;
	case AMPI_SEND:
	    rc=BW_AMPI_Send(buf,count,datatype,dest,tag,pairedWith,comm);
	    break;
	case AMPI_RECV:
	    rc=BW_AMPI_Recv(buf,count,datatype,dest,tag,pairedWith,comm,&status);
	    break;
	case AMPI_ISEND:
	    rc=BW_AMPI_Isend(buf,count,datatype,dest,tag,pairedWith,comm,&request);
	    break;
	case AMPI_IRECV:
	    rc=BW_AMPI_Irecv(buf,count,datatype,dest,tag,pairedWith,comm,&request);
	    break;
	case AMPI_WIN_CREATE:
	    std::cout << "Win_create start" << std::endl;
            rc=BW_AMPI_Win_create( base, size, disp_unit,
		 info, comm, &win );
	    std::cout << "Win_create end" << std::endl;
	    break;
	case AMPI_WIN_FREE:
	    std::cout << "Win_free start" << std::endl;
	    rc=BW_AMPI_Win_free( &win );
	    std::cout << "Win_free end" << std::endl;
	    break;
	case AMPI_WIN_FENCE:
	    std::cout << "Win_fence start" << std::endl;
	    rc=BW_AMPI_Win_fence( assert, win );
	    std::cout << "Win_fence end" << std::endl;
	    break;
	case AMPI_GET:
	    std::cout << "Get start" << std::endl;
	    rc=BW_AMPI_Get(origin_addr, origin_count, origin_datatype, target_rank,
		 target_disp, target_count, target_datatype, win);
	    std::cout << "Get end" << std::endl;
	    break;
	case AMPI_PUT:
	    rc=BW_AMPI_Put(origin_addr, origin_count, origin_datatype, target_rank,
		 target_disp, target_count, target_datatype, win);
	    break;
	default:
	    assert(0);
	    break;
    }

}

void ampi_create_dummies(void *buf, int *size) {
    type *values=static_cast<type*>(buf);

    for(int i=0;i<*size;++i) {
	type &dummy=values[i];
	dummy=0;
	global_tape->register_variable(dummy);
    }
}



void ADTOOL_AMPI_pushSRinfo(void* buf, 
			    int count,
			    MPI_Datatype datatype, 
			    int endPoint, 
			    int tag,
			    enum AMPI_PairedWith_E pairedWith,
			    MPI_Comm comm) { 
  INT64 *idx=new INT64[count];
  int tmp=0;
  for(int i=0;i<count;i++) {
      ampi_get_idx(buf,&i,&tmp);
      idx[i]=tmp;
      TAPE_AMPI_push_int(tmp);
  }
  TAPE_AMPI_push_ptr(idx);
  TAPE_AMPI_push_int(count);
  TAPE_AMPI_push_MPI_Datatype(datatype);
  TAPE_AMPI_push_int(endPoint);
  TAPE_AMPI_push_int(tag);
  TAPE_AMPI_push_int(pairedWith);
  TAPE_AMPI_push_MPI_Comm(comm);
}

void ADTOOL_AMPI_popSRinfo(void** buf, 
			   int* count,
			   MPI_Datatype* datatype, 
			   int* endPoint, 
			   int* tag,
			   AMPI_PairedWith_E* pairedWith,
			   MPI_Comm* comm,
			   void **idx) {
  int tmp=0;
  void *ptr;
  TAPE_AMPI_pop_MPI_Comm(comm);
  TAPE_AMPI_pop_int((int*)pairedWith);
  TAPE_AMPI_pop_int(tag);
  TAPE_AMPI_pop_int(endPoint);
  TAPE_AMPI_pop_MPI_Datatype(datatype);
  TAPE_AMPI_pop_int(count);
  //idx=new void*;
  *idx=0;
  TAPE_AMPI_pop_ptr(&ptr);
  *idx=ptr;
  INT64 *idx_=(INT64*) *idx;
  //idx=new void*[1];
  //*idx=ptr;

  *buf=new double[*count];
  double *ptr_buf = (double*) *buf;
  for(int i=0;i<*count;i++) {
      TAPE_AMPI_pop_int(&tmp);
      ampi_get_adj(&tmp,&ptr_buf[i]);
  }
  //void* test;
  //std::cout << "lala " << std::endl;
  //for(int i=*count-1;i>=0;i++) {
      //TAPE_AMPI_pop_int(&tmp);
      //ampi_get_adj(&tmp,&(*buf[i]));
      //ampi_get_adj(&tmp,test);
  //}
}

void ADTOOL_AMPI_push_CallCode(enum AMPI_CallCode_E thisCall) { 
  
  int tmp=0;
  tmp=thisCall;
  external_function_ampi_data *ext_func_args=new external_function_ampi_data;
  //ampi_create_tape_entry(&tmp);
  switch(thisCall) { 
  case AMPI_WAIT:
      tmp=AMPI_WAIT;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_SEND:
      tmp=AMPI_SEND;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_RECV:
      tmp=AMPI_RECV;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_ISEND:
      tmp=AMPI_ISEND;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_IRECV:
      tmp=AMPI_IRECV;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_WIN_CREATE:
      tmp=AMPI_WIN_CREATE;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_WIN_FREE:
      tmp=AMPI_WIN_FREE;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_WIN_FENCE:
      tmp=AMPI_WIN_FENCE;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_PUT:
      tmp=AMPI_PUT;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  case AMPI_GET:
      tmp=AMPI_GET;
      ext_func_args->opcode=tmp;
      global_tape->register_external_function(&ampi_tape_wrapper,ext_func_args);
      break;
  default:
      assert(0);
      break;
  } 
}

void ADTOOL_AMPI_pop_CallCode(enum AMPI_CallCode_E *thisCall) { 
  assert(0);
}

void ADTOOL_AMPI_push_AMPI_Request(struct AMPI_Request_S  *ampiRequest) { 
  ADTOOL_AMPI_pushSRinfo(ampiRequest->buf, 
		         ampiRequest->count,
			 ampiRequest->datatype,
			 ampiRequest->endPoint,
			 ampiRequest->tag,
			 ampiRequest->pairedWith,
			 ampiRequest->comm);
  TAPE_AMPI_push_MPI_Request(ampiRequest->tracedRequest);
  TAPE_AMPI_push_int(ampiRequest->origin);
  TAPE_AMPI_push_ptr(ampiRequest->idx);
}

void ADTOOL_AMPI_pop_AMPI_Request(struct AMPI_Request_S  *ampiRequest) { 
  TAPE_AMPI_pop_ptr(&ampiRequest->idx);
  void *idx;
  TAPE_AMPI_pop_int((int*)&(ampiRequest->origin));
  TAPE_AMPI_pop_MPI_Request(&(ampiRequest->tracedRequest));
  ADTOOL_AMPI_popSRinfo(&(ampiRequest->adjointBuf), 
			&(ampiRequest->count),
			&(ampiRequest->datatype),
			&(ampiRequest->endPoint),
			&(ampiRequest->tag),
			&(ampiRequest->pairedWith),
			&(ampiRequest->comm),
			&idx);
}

void ADTOOL_AMPI_push_request(MPI_Request request) { 
  TAPE_AMPI_push_MPI_Request(request);
} 

MPI_Request ADTOOL_AMPI_pop_request() {
  MPI_Request r;
  TAPE_AMPI_pop_MPI_Request(&r);
  return r;
}

void * ADTOOL_AMPI_rawData(void* activeData,int *size) { 
  AMPI_ht_el *ht_req=NULL;
  double *buf=NULL;
  HASH_FIND_PTR(AMPI_ht,activeData,ht_req);
  if(!ht_req) {
    buf=new double[*size];
    AMPI_ht_el *ht_el=new AMPI_ht_el;
    ht_el->key=activeData;
    ht_el->ptr_buf=buf;
    HASH_ADD_PTR(AMPI_ht,key,ht_el);
  }
  else {
    buf=ht_req->ptr_buf;
  }
  type *tmp=(type*) activeData;
  int i=0;
  for(i=0;i<*size;i=i+1) ampi_get_val(activeData,&i,&buf[i]);
  return buf;
}

void ADTOOL_AMPI_writeData(void* activeData,int *size) { 
    AMPI_ht_el *ht_req=NULL;
    double *buf=NULL;
    HASH_FIND_PTR(AMPI_ht,&activeData,ht_req);
    if(!ht_req) {
	std::cout << "Error: Buffer not found" << std::endl;
    }
    else {
	buf=ht_req->ptr_buf;
    }
    type *tmp=(type*) activeData;
    int i=0;
    for(i=0;i<*size;i=i+1) {
      ampi_set_val(activeData,&i,&buf[i]);
      //std::cout << "buf[i] " << buf[i] << std::endl;
    }
}


void * ADTOOL_AMPI_rawAdjointData(void* activeData) {
    return activeData;
}

void ADTOOL_AMPI_mapBufForAdjoint(struct AMPI_Request_S  *ampiRequest,
				  void* buf) { 
  INT64 *idx=new INT64[ampiRequest->count];
  int tmp=0;
  for(int i=0;i<ampiRequest->count;i++) {
      ampi_get_idx(buf,&i,&tmp);
      idx[i]=tmp;
  }
  ampiRequest->buf=buf;
  ampiRequest->idx=idx;
}

void ADTOOL_AMPI_setBufForAdjoint(struct AMPI_Request_S  *ampiRequest,
				  void* buf) { 
  /* do nothing */
}

void ADTOOL_AMPI_getAdjointCount(int *count,
				 MPI_Datatype datatype) { 
}

void ADTOOL_AMPI_setAdjointCount(struct AMPI_Request_S  *ampiRequest) { 
  /* for now we keep the count as is but for example in vector mode one would have to multiply by vector length */
  ampiRequest->adjointCount=ampiRequest->count;
  ampiRequest->adjointBuf=
      ADTOOL_AMPI_allocateTempBuf(ampiRequest->adjointCount,
          ampiRequest->datatype,
          ampiRequest->comm);
  INT64 *idx=(INT64*) ampiRequest->idx;
  double *x=(double*) ampiRequest->adjointBuf;
  for(int i=0;i<ampiRequest->adjointCount;i++) {
    if(idx[i]!=0) x[i] = dco::a1s::global_tape->_adjoint(idx[i]);
  }
}

void ADTOOL_AMPI_setAdjointCountAndTempBuf(struct AMPI_Request_S *ampiRequest) { 
  ADTOOL_AMPI_setAdjointCount(ampiRequest);
  ampiRequest->adjointTempBuf=
      ADTOOL_AMPI_allocateTempBuf(ampiRequest->adjointCount,
          ampiRequest->datatype,
          ampiRequest->comm);
  INT64 *idx=(INT64*) ampiRequest->idx;
  double *x=(double*) ampiRequest->adjointTempBuf;
  for(int i=0;i<ampiRequest->adjointCount;i++) {
    if(idx[i]!=0) x[i] = dco::a1s::global_tape->_adjoint(idx[i]);
  }
}

void ADTOOL_AMPI_setWinAdjointCount(AMPI_WinRequest  *winRequest) { 
  /* for now we keep the count as is but for example in vector mode one would have to multiply by vector length */
  winRequest->adjointCount=winRequest->origin_count;
  winRequest->adjointBuf=malloc(winRequest->adjointCount*sizeof(double));
  //winRequest->adjointBuf=
      //ADTOOL_AMPI_allocateTempBuf(winRequest->adjointCount,
          //winRequest->datatype,
          //winRequest->comm);
  INT64 *idx=(INT64*) winRequest->idx;
  double *x=(double*) winRequest->adjointBuf;
  for(int i=0;i<winRequest->adjointCount;i++) {
    if(idx[i]!=0) x[i] = dco::a1s::global_tape->_adjoint(idx[i]);
  }
}

void ADTOOL_AMPI_setWinAdjointCountAndTempBuf(AMPI_WinRequest *winRequest) { 
  winRequest->adjointCount=winRequest->origin_count;
  printf("BW adjointCount: %d\n", winRequest->adjointCount);

  winRequest->adjointTempBuf=malloc(winRequest->adjointCount*sizeof(double));
  //winRequest->adjointTempBuf=
      //ADTOOL_AMPI_allocateTempBuf(win?gRequest->adjointCount,
          //win?gRequest->datatype,
          //win?gRequest->comm);
	 
  INT64 *idx=(INT64*) winRequest->idx;
  double *x=(double*) winRequest->adjointTempBuf;
  for(int i=0;i<winRequest->adjointCount;i++) {
    x[i]=0;
    if(idx[i]!=0) x[i] = dco::a1s::global_tape->_adjoint(idx[i]);
    printf("BW adj: %f\n", x[i]);
  }
}

void* ADTOOL_AMPI_allocateTempBuf(int adjointCount,
                                  MPI_Datatype dataType,
                                  MPI_Comm comm) {
  size_t s=0;
  void* buf;
  if(dataType==MPI_DOUBLE) s=sizeof(double);
  else if(dataType==MPI_FLOAT) s=sizeof(float);
  else MPI_Abort(comm, MPI_ERR_TYPE);
  buf=malloc(adjointCount*s);
  assert(buf);
  return buf;
}

void ADTOOL_AMPI_releaseAdjointTempBuf(void *tempBuf) {
  free(tempBuf);
}

//void ADTOOL_AMPI_adjointIncrement(int adjointCount,
    //MPI_Datatype datatype,
    //MPI_Comm comm,
    //void* target,
    //void* adjointTarget,
    //void* checkAdjointTarget,
    //void *source_,
    //void *idx_) {
  ////for (unsigned int i=0; i<adjointCount; ++i) ((revreal*)(adjointTarget))[i]+=((revreal*)(source))[i];
  //INT64 *idx=(INT64*)idx_;
  //double *source=(double*)source_;
  //for(int i=0;i<adjointCount;i++) {
    //if(idx[i]!=0) {
      //const_cast<double&>(dco::a1s::global_tape->_adjoint(idx[i])) += source[i];
    //}
  //}


//}

void ADTOOL_AMPI_incrementAdjoint(int adjointCount,
                                  MPI_Datatype datatype,
                                  MPI_Comm comm,
                                  void* target,
                                  void *source_,
				  void *idx_) {
  //for (unsigned int i=0; i<adjointCount; ++i) ((revreal*)(adjointTarget))[i]+=((revreal*)(source))[i];
  INT64 *idx=(INT64*)idx_;
  double *source=(double*)source_;
  for(int i=0;i<adjointCount;i++) {
    if(idx[i]!=0) {
      const_cast<double&>(dco::a1s::global_tape->_adjoint(idx[i])) += source[i];
    }
  }

    
}

//void ADTOOL_AMPI_adjointNullify(int adjointCount,
                                //MPI_Datatype datatype,
                                //MPI_Comm comm,
                                //void* target,
                                //void* adjointTarget,
                                //void* checkAdjointTarget) {
    ////for (unsigned int i=0; i<adjointCount; ++i) ((revreal*)(adjointTarget))[i]=0.0;
//}

void ADTOOL_AMPI_nullifyAdjoint(int adjointCount,
                                MPI_Datatype datatype,
                                MPI_Comm comm,
                                void* target) {
    //for (unsigned int i=0; i<adjointCount; ++i) ((revreal*)(adjointTarget))[i]=0.0;
}


// tracing 

int AMPI_Send(void* buf,
              int count,
              MPI_Datatype datatype,
              int src,
              int tag,
              AMPI_PairedWith pairedWith,
              MPI_Comm comm) {
  return FW_AMPI_Send(buf,
                      count,
                      datatype,
                      src,
                      tag,
                      pairedWith,
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
    type *values=static_cast<type*>(buf);

  for(int i=0;i<count;++i) {
      type &dummy=values[i];
      dummy=0;
      global_tape->register_variable(dummy);
  }
  return FW_AMPI_Recv(buf,
                      count,
                      datatype,
                      src,
                      tag,
                      pairedWith,
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
  return FW_AMPI_Isend(buf,
                       count,
                       datatype,
                       dest,
                       tag,
                       pairedWith,
                       comm,
                       request);
}

int AMPI_Irecv (void* buf,
                int count,
                MPI_Datatype datatype,
                int src,
                int tag,
                AMPI_PairedWith pairedWith,
                MPI_Comm comm,
                AMPI_Request* request) {
    type *values=static_cast<type*>(buf);

  for(int i=0;i<count;++i) {
      type &dummy=values[i];
      dummy=0;
      global_tape->register_variable(dummy);
  }
  return FW_AMPI_Irecv(buf,
                       count,
                       datatype,
                       src,
                       tag,
                       pairedWith,
                       comm,
                       request);
}

int AMPI_Wait(AMPI_Request *request,
              MPI_Status *status) {
  return FW_AMPI_Wait(request,
                      status);
}

int AMPI_Win_create( void *base,
		     MPI_Aint size,
		     int disp_unit,
		     MPI_Info info,
		     MPI_Comm comm,
		     AMPI_Win *win
		   ) 
{
  return FW_AMPI_Win_create( base,
      size,
      disp_unit,
      info,
      comm,
      win
      ); 
}

int AMPI_Win_free( AMPI_Win *win )
{
  return FW_AMPI_Win_free(win);
}

int AMPI_Win_fence( int assert,
                    AMPI_Win win
                  )
{
return FW_AMPI_Win_fence( assert,
                    win
                  );
}

int AMPI_Get( void *origin_addr,
	      int origin_count,
	      MPI_Datatype origin_datatype, 
	      int target_rank,
	      MPI_Aint target_disp,
	      int target_count,
	      MPI_Datatype target_datatype,
	      AMPI_Win win
            ) 
{
  type *values=static_cast<type*>(origin_addr);
  for(int i=0;i<origin_count;++i) {
    type &dummy=values[i];
    dummy=0;
    global_tape->register_variable(dummy);
  }
  return FW_AMPI_Get( origin_addr,
      origin_count,
      origin_datatype, 
      target_rank,
      target_disp,
      target_count,
      target_datatype,
      win
      ); 
}

int AMPI_Put( void *origin_addr,
	      int origin_count,
	      MPI_Datatype origin_datatype, 
	      int target_rank,
	      MPI_Aint target_disp,
	      int target_count,
	      MPI_Datatype target_datatype,
	      AMPI_Win win
            ) 
{
  type *values=static_cast<type*>(origin_addr);
  for(int i=0;i<origin_count;++i) {
    type &dummy=values[i];
    dummy=0;
    global_tape->register_variable(dummy);
  }
  //return FW_AMPI_Put( origin_addr,
      //origin_count,
      //origin_datatype, 
      //target_rank,
      //target_disp,
      //target_count,
      //target_datatype,
      //win
      //); 
}


void ADTOOL_AMPI_mapWinBufForAdjoint(AMPI_WinRequest  *winRequest,
				  void* buf) { 
  std::cout << "Hello" << std::endl;
  INT64 *idx=new INT64[winRequest->origin_count];
  int tmp=0;
  for(int i=0;i<winRequest->origin_count;i++) {
      ampi_get_idx(buf,&i,&tmp);
      idx[i]=tmp;
      std::cout << "idx: " << idx[i] << std::endl;
  }
  winRequest->origin_addr=buf;
  winRequest->idx=idx;
}

void * ADTOOL_AMPI_createWinMap(void *buf, MPI_Aint size) {
  double factor=(double) sizeof(double)/(double) sizeof(type);
  MPI_Aint mapsize=(MPI_Aint) (factor * (double) size);
  double *ret_buf=(double*) malloc(mapsize);
  int count=mapsize/sizeof(double);
  int i=0;
  for(i=0;i<count;i=i+1) {
    ampi_get_val(buf,&i,&ret_buf[i]);
  }
  return ret_buf;
  
}


void ADTOOL_AMPI_writeWinData(void* map, void *base, MPI_Aint size) { 
  int count=size/sizeof(double);
  int i=0;


  double *buf=(double *) map;
  for(i=0;i<count;i=i+1) {
    ampi_get_val(base,&i,&buf[i]);
  }
}

MPI_Aint ADTOOL_AMPI_getWinSize(MPI_Aint size) {
  double factor=(double) sizeof(double)/(double) sizeof(type);
  return (MPI_Aint) (factor * (double) size);
}

void ADTOOL_AMPI_syncAdjointWin(AMPI_Win *win) {

  printf("SYNC ADJOINT BEGIN\n");

  INT64 *idx=(INT64 *) win->idx;
  printf("IDX: %ld\n", idx[0]);
  double *source=(double*) *win->map;
  printf("MAP: %f\n", source[0]);
  printf("SIZE: %ld\n", win->size);
  MPI_Aint adjointCount=win->size/sizeof(double);
  printf("COUNT: %d\n", adjointCount);
  for(MPI_Aint i=0;i<adjointCount;i++) {
    if(idx[i]!=0) {
      const_cast<double&>(dco::a1s::global_tape->_adjoint(idx[i])) += source[i];
      source[i]=0;
    }
  }

  printf("SYNC ADJOINT END\n");

}

void ADTOOL_AMPI_push_AMPI_WinRequest(AMPI_WinRequest *winRequest) { 
  INT64 tmp=0;
  int test=7;
  int count=winRequest->origin_count;
  for(int i=0;i<count;i++) {
    ampi_get_idx(winRequest->origin_addr,&i,&tmp);
    std::cout << "FW push AMPI_WinRequest idx: " << tmp << std::endl;
    TAPE_AMPI_push_int(tmp);
  }

  TAPE_AMPI_push_ptr(winRequest->origin_addr); 
  TAPE_AMPI_push_int(winRequest->origin_count); 
  TAPE_AMPI_push_MPI_Datatype(winRequest->origin_datatype); 
  TAPE_AMPI_push_int(winRequest->target_rank); 
  TAPE_AMPI_push_MPI_Aint(winRequest->target_disp); 
  TAPE_AMPI_push_int(winRequest->target_count); 
  TAPE_AMPI_push_MPI_Datatype(winRequest->target_datatype); 
  //TAPE_AMPI_push_ptr(winRequest->idx); 

}

void ADTOOL_AMPI_pop_AMPI_WinRequest(AMPI_WinRequest *winRequest) { 
  int test=0;
  //TAPE_AMPI_pop_ptr(&winRequest->idx); 
  TAPE_AMPI_pop_MPI_Datatype(&winRequest->target_datatype); 
  TAPE_AMPI_pop_int(&winRequest->target_count); 
  TAPE_AMPI_pop_MPI_Aint(&winRequest->target_disp); 
  TAPE_AMPI_pop_int(&winRequest->target_rank); 
  TAPE_AMPI_pop_MPI_Datatype(&winRequest->origin_datatype); 
  TAPE_AMPI_pop_int(&winRequest->origin_count); 
  TAPE_AMPI_pop_ptr(&winRequest->origin_addr); 

  int count=winRequest->origin_count;
  INT64 *idx=new int[count];
  for(int i=count-1;i>=0;i--) {
    TAPE_AMPI_pop_int(&idx[i]);
    std::cout << "BW pop AMPI_WinRequest idx: " << idx[i] << std::endl;
  }
  winRequest->idx=(void *) idx;
}

void ADTOOL_AMPI_push_AMPI_Win(AMPI_Win *win) { 
  INT64 tmp=0;
  printf("FW PUSH win ptr: %p\n", win->plainWindow);
  printf("FW PUSH win ptr: %p\n", win->map);
  //TAPE_AMPI_push_MPI_Win(win->plainWindow);
  int count=win->size/sizeof(double);
  for(int i=0;i<count;i++) {
    ampi_get_idx(win->base,&i,&tmp);
    std::cout << "FW push AMPI Win idx: " << tmp << std::endl;
    TAPE_AMPI_push_int(tmp);
  }

  TAPE_AMPI_push_ptr(win->plainWindow);
  TAPE_AMPI_push_ptr(win->map);
  TAPE_AMPI_push_ptr(win->base);
  TAPE_AMPI_push_MPI_Aint(win->size);
  TAPE_AMPI_push_int(win->num_reqs);
  TAPE_AMPI_push_ptr(win->req_stack);
  TAPE_AMPI_push_MPI_Aint(win->disp);
  TAPE_AMPI_push_MPI_Comm(win->comm);
}

void ADTOOL_AMPI_pop_AMPI_Win(AMPI_Win *win) { 
  TAPE_AMPI_pop_MPI_Comm(&win->comm);
  TAPE_AMPI_pop_MPI_Aint(&win->disp);
  TAPE_AMPI_pop_ptr((void**) &win->req_stack);
  TAPE_AMPI_pop_int(&win->num_reqs);
  TAPE_AMPI_pop_MPI_Aint(&win->size);
  TAPE_AMPI_pop_ptr(&win->base);
  TAPE_AMPI_pop_ptr((void**) &win->map);
  TAPE_AMPI_pop_ptr((void**) &win->plainWindow);
  printf("BW POP map ptr: %p\n", win->map);
  printf("BW POP win ptr: %p\n", win->plainWindow);
  int count=win->size/sizeof(double);
  INT64 *idx=new int[count];
  for(int i=count-1;i>=0;i--) {
    TAPE_AMPI_pop_int(&idx[i]);
    std::cout << "BW pop AMPI_Win idx: " << idx[i] << std::endl;
  }
  win->idx=(void *) idx;
}

void ADTOOL_AMPI_pushOSinfo(void* buf, 
			    int count,
			    MPI_Datatype datatype, 
			    int endPoint, 
			    int tag,
			    enum AMPI_PairedWith_E pairedWith,
			    MPI_Comm comm) { 
}

void ADTOOL_AMPI_popOSinfo(void** buf, 
			   int* count,
			   MPI_Datatype* datatype, 
			   int* endPoint, 
			   int* tag,
			   AMPI_PairedWith_E* pairedWith,
			   MPI_Comm* comm,
			   void **idx) {
}
AMPI_Activity ADTOOL_AMPI_isActiveType(MPI_Datatype datatype) {
    if (datatype==AMPI_ADOUBLE || datatype==AMPI_AFLOAT) return AMPI_ACTIVE;
    return AMPI_PASSIVE;
}

void ADTOOL_AMPI_setupTypes() {
    AMPI_ADOUBLE=MPI_DOUBLE;
    AMPI_AFLOAT=MPI_FLOAT;
}

void ADTOOL_AMPI_cleanupTypes() {
}

MPI_Datatype ADTOOL_AMPI_FW_rawType(MPI_Datatype datatype) {
  return datatype;
}
MPI_Datatype ADTOOL_AMPI_BW_rawType(MPI_Datatype datatype) {
  return datatype;
}

int AMPI_Init_NT(int* argc,
		 char*** argv) {
  int rc;
  rc=MPI_Init(argc,
              argv);
  ADTOOL_AMPI_setupTypes();
  //ourADTOOL_AMPI_FPCollection.pushBcastInfo_fp=&ADTOOL_AMPI_pushBcastInfo;
  //ourADTOOL_AMPI_FPCollection.popBcastInfo_fp=&ADTOOL_AMPI_popBcastInfo;
  //ourADTOOL_AMPI_FPCollection.pushDoubleArray_fp=&ADTOOL_AMPI_pushDoubleArray;
  //ourADTOOL_AMPI_FPCollection.popDoubleArray_fp=&ADTOOL_AMPI_popDoubleArray;
  //ourADTOOL_AMPI_FPCollection.pushReduceInfo_fp=&ADTOOL_AMPI_pushReduceInfo; 
  //ourADTOOL_AMPI_FPCollection.popReduceCountAndType_fp=&ADTOOL_AMPI_popReduceCountAndType;
  //ourADTOOL_AMPI_FPCollection.popReduceInfo_fp=&ADTOOL_AMPI_popReduceInfo; 
  ourADTOOL_AMPI_FPCollection.pushSRinfo_fp=&ADTOOL_AMPI_pushSRinfo;
  ourADTOOL_AMPI_FPCollection.popSRinfo_fp=&ADTOOL_AMPI_popSRinfo;
  ourADTOOL_AMPI_FPCollection.pushOSinfo_fp=&ADTOOL_AMPI_pushOSinfo;
  ourADTOOL_AMPI_FPCollection.popOSinfo_fp=&ADTOOL_AMPI_popOSinfo;
  //ourADTOOL_AMPI_FPCollection.pushGSinfo_fp=&ADTOOL_AMPI_pushGSinfo;
  //ourADTOOL_AMPI_FPCollection.popGScommSizeForRootOrNull_fp=&ADTOOL_AMPI_popGScommSizeForRootOrNull;
  //ourADTOOL_AMPI_FPCollection.popGSinfo_fp=&ADTOOL_AMPI_popGSinfo;
  //ourADTOOL_AMPI_FPCollection.pushGSVinfo_fp=&ADTOOL_AMPI_pushGSVinfo;
  //ourADTOOL_AMPI_FPCollection.popGSVinfo_fp=&ADTOOL_AMPI_popGSVinfo;
  //
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
  //ourADTOOL_AMPI_FPCollection.push_comm_fp=&ADTOOL_AMPI_push_comm;
  //ourADTOOL_AMPI_FPCollection.pop_comm_fp=&ADTOOL_AMPI_pop_comm;
  ourADTOOL_AMPI_FPCollection.rawData_fp=&ADTOOL_AMPI_rawData;
  //ourADTOOL_AMPI_FPCollection.rawDataV_fp=&ADTOOL_AMPI_rawDataV;
  //ourADTOOL_AMPI_FPCollection.packDType_fp=&ADTOOL_AMPI_packDType;
  //ourADTOOL_AMPI_FPCollection.unpackDType_fp=&ADTOOL_AMPI_unpackDType;
  ourADTOOL_AMPI_FPCollection.writeData_fp=&ADTOOL_AMPI_writeData;
  //ourADTOOL_AMPI_FPCollection.writeDataV_fp=&ADTOOL_AMPI_writeDataV;
  ourADTOOL_AMPI_FPCollection.rawAdjointData_fp=&ADTOOL_AMPI_rawAdjointData;
  //ourADTOOL_AMPI_FPCollection.Turn_fp=&ADTOOL_AMPI_Turn;
  ourADTOOL_AMPI_FPCollection.mapBufForAdjoint_fp=&ADTOOL_AMPI_mapBufForAdjoint;
  ourADTOOL_AMPI_FPCollection.mapWinBufForAdjoint_fp=&ADTOOL_AMPI_mapWinBufForAdjoint;
  ourADTOOL_AMPI_FPCollection.setBufForAdjoint_fp=&ADTOOL_AMPI_setBufForAdjoint;
  ourADTOOL_AMPI_FPCollection.getAdjointCount_fp=&ADTOOL_AMPI_getAdjointCount;
  ourADTOOL_AMPI_FPCollection.setAdjointCount_fp=&ADTOOL_AMPI_setAdjointCount;
  ourADTOOL_AMPI_FPCollection.setWinAdjointCount_fp=&ADTOOL_AMPI_setWinAdjointCount;
  ourADTOOL_AMPI_FPCollection.setAdjointCountAndTempBuf_fp=&ADTOOL_AMPI_setAdjointCountAndTempBuf;
  ourADTOOL_AMPI_FPCollection.setWinAdjointCountAndTempBuf_fp=&ADTOOL_AMPI_setWinAdjointCountAndTempBuf;
  ourADTOOL_AMPI_FPCollection.allocateTempBuf_fp=&ADTOOL_AMPI_allocateTempBuf;
  ourADTOOL_AMPI_FPCollection.releaseAdjointTempBuf_fp=&ADTOOL_AMPI_releaseAdjointTempBuf;
  ourADTOOL_AMPI_FPCollection.incrementAdjoint_fp=&ADTOOL_AMPI_incrementAdjoint;
  //ourADTOOL_AMPI_FPCollection.adjointMultiply_fp=&ADTOOL_AMPI_adjointMultiply;
  //ourADTOOL_AMPI_FPCollection.adjointDivide_fp=&ADTOOL_AMPI_adjointDivide;
  //ourADTOOL_AMPI_FPCollection.adjointEquals_fp=&ADTOOL_AMPI_adjointEquals;
  ourADTOOL_AMPI_FPCollection.nullifyAdjoint_fp=&ADTOOL_AMPI_nullifyAdjoint;
  ourADTOOL_AMPI_FPCollection.setupTypes_fp=&ADTOOL_AMPI_setupTypes;
  ourADTOOL_AMPI_FPCollection.FW_rawType_fp=&ADTOOL_AMPI_FW_rawType;
  ourADTOOL_AMPI_FPCollection.BW_rawType_fp=&ADTOOL_AMPI_BW_rawType;
  ourADTOOL_AMPI_FPCollection.isActiveType_fp=&ADTOOL_AMPI_isActiveType;
  ourADTOOL_AMPI_FPCollection.createWinMap_fp=&ADTOOL_AMPI_createWinMap;
  ourADTOOL_AMPI_FPCollection.writeWinData_fp=&ADTOOL_AMPI_writeWinData;
  ourADTOOL_AMPI_FPCollection.getWinSize_fp=&ADTOOL_AMPI_getWinSize;
  ourADTOOL_AMPI_FPCollection.syncAdjointWin_fp=&ADTOOL_AMPI_syncAdjointWin;
  //ourADTOOL_AMPI_FPCollection.allocateTempActiveBuf_fp=&ADTOOL_AMPI_allocateTempActiveBuf;
  //ourADTOOL_AMPI_FPCollection.releaseTempActiveBuf_fp=&ADTOOL_AMPI_releaseTempActiveBuf;
  //ourADTOOL_AMPI_FPCollection.copyActiveBuf_fp=&ADTOOL_AMPI_copyActiveBuf;
  
  return rc;
}

