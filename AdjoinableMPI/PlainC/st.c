/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <mpi.h>
#include "ampi/userIF/st.h"

int AMPI_Wait_ST(AMPI_Request *request,
		 void*  buf,
		 MPI_Status *status) { 
  return MPI_Wait(
#ifdef AMPI_FORTRANCOMPATIBLE
		   request
#else 
		   &(request->plainRequest)
#endif 
		   ,status);
}
