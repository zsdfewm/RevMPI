/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include "dco.hpp"
#include "ampi_tape.hpp"

//#define INT64 int

using namespace dco::a1s;

extern "C" {
  //forward declare von AMPI

#ifndef DCO_AMPI
  void ampi_interpret_tape() {}
#endif


  void ampi_get_val(void *buf, int *i, double *x) {
    *x=static_cast<type*>(buf)[*i]._value();
  }
  void ampi_set_val(void* buf, int *i, double *v) {
    type &dummy= static_cast<type*>(buf)[*i];
    *const_cast<double*>(&(dummy._value())) = *v;
  }

  void ampi_get_idx(void *buf, int *i, INT64 *idx) {
    type &var = static_cast<type*>(buf)[*i];
    if(!var._data()._is_registered()) {
      *idx=0;
    }
    else {
      *idx = var._data().tape_index();
    }

  }


  void ampi_get_adj(INT64 *idx, double *x) {
    if(*idx!=0) *x = dco::a1s::global_tape->_adjoint(*idx);
  }
  void ampi_set_adj(INT64 *idx, double *x) {
    if(*idx!=0) const_cast<double&>(dco::a1s::global_tape->_adjoint(*idx)) += *x;
  }

  void ampi_tape_wrapper(tape &caller, const tape::interpretation_settings &settings, dco::a1s::tape::external_function_base_data *userdata) {
    ampi_interpret_tape();
  }

  void ampi_create_tape_entry(int *i) {
    global_tape->register_external_function(&ampi_tape_wrapper, 0);
  }

  void ampi_create_dummies(void *buf, int *size) {
    type *values=static_cast<type*>(buf);
    
    for(int i=0;i<*size;++i) {
      type &dummy=values[i];
      dummy=0;
      global_tape->register_variable(dummy);
    } 
  }

}
