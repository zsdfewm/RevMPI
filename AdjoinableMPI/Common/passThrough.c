/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <mpi.h>
#include "ampi/userIF/passThrough.h"

int AMPI_Comm_size(MPI_Comm comm, 
		   int *size) {
  return MPI_Comm_size(comm,
		       size);
}

int AMPI_Comm_rank(MPI_Comm comm, 
		   int *rank) {
  return MPI_Comm_rank(comm,
		       rank);
}

int AMPI_Get_processor_name(char *name, 
			    int *resultlen ) { 
  return MPI_Get_processor_name(name,
				resultlen);
}

