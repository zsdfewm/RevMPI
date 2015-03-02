/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/

#include <mpi.h>

int AMPI_Init_NT(int* argc, 
	      char*** argv) { 
  int rc = MPI_Init(argc, argv);
  return rc ;
}

int AMPI_Finalize_NT(void) { 
  return MPI_Finalize();
}


int AMPI_Buffer_attach_NT(void *buffer, 
			  int size) { 
  return MPI_Buffer_attach(buffer,
			   size);

}

int AMPI_Buffer_detach_NT(void *buffer, 
			  int *size){ 
  return MPI_Buffer_detach(buffer,
			   size);
}

int AMPI_Op_create_NT(MPI_User_function *function,
		      int commute,
		      MPI_Op *op) {
  int rc;
  rc = MPI_Op_create(function,
		     commute,
		     op);
  return rc;
}

int AMPI_Op_free_NT(MPI_Op *op) {
  return MPI_Op_free(op);
}

int AMPI_Type_create_struct_NT(int count,
			       int array_of_blocklengths[],
			       MPI_Aint array_of_displacements[],
			       MPI_Datatype array_of_types[],
			       MPI_Datatype *newtype) {
  int rc;
  rc = MPI_Type_create_struct (count,
			       array_of_blocklengths,
			       array_of_displacements,
			       array_of_types,
			       newtype);
  return rc;
}
