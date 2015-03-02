/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <mpi.h>
#include "ampi/userIF/nt.h"
#include "ampi/adTool/support.h"
#include <assert.h>

struct ADTOOL_AMPI_FPCollection ourADTOOL_AMPI_FPCollection;

int AMPI_Finalize_NT(void) {
  releaseDTypeData();
  releaseUOpData();
  ADTOOL_AMPI_cleanupTypes();
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

int AMPI_Type_contiguous_NT(int count,
			    MPI_Datatype oldtype,
			    MPI_Datatype* newtype) {
  int rc;
  rc = MPI_Type_contiguous (count,
			    oldtype,
			    newtype);
  assert(rc==MPI_SUCCESS);
  MPI_Datatype type, temp_packed_type, packed_type;
  MPI_Aint array_of_displacements[1] = {(MPI_Aint)0};
  int s=0, is_active, dt_idx;
  MPI_Aint p_mapsize, extent, lb;
  is_active = (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(oldtype)==AMPI_ACTIVE;
  dt_idx = derivedTypeIdx(oldtype);
  derivedTypeData* dtd = getDTypeData();
  if (is_active) {
    type = MPI_DOUBLE;
    s = sizeof(double);
  }
  else if (isDerivedType(dt_idx)) {
    type = dtd->packed_types[dt_idx];
    s = dtd->p_extents[dt_idx];
  }
  else {
    type = oldtype;
    if (oldtype==MPI_DOUBLE) s = sizeof(double);
    else if (oldtype==MPI_INT) s = sizeof(int);
    else if (oldtype==MPI_FLOAT) s = sizeof(float);
    else if (oldtype==MPI_CHAR) s = sizeof(char);
    else assert(0);
  }
  p_mapsize = count*s;
  MPI_Type_get_extent(*newtype,&lb,&extent);
  rc = MPI_Type_contiguous (count,
			    type,
			    &temp_packed_type);
  assert(rc==MPI_SUCCESS);
  rc = MPI_Type_create_resized (temp_packed_type,
				0,
				(MPI_Aint)p_mapsize,
				&packed_type);
  addDTypeData(dtd,
	       1,
	       &count,
	       array_of_displacements,
	       &oldtype,
	       lb,
	       extent,
	       &count,
	       array_of_displacements,
	       &type,
	       p_mapsize,
	       newtype,
	       &packed_type);
  return rc;
}

int AMPI_Type_create_struct_NT(int count,
			       int array_of_blocklengths[],
			       MPI_Aint array_of_displacements[],
			       MPI_Datatype array_of_types[],
			       MPI_Datatype *newtype) {
  int i, rc;
  rc = MPI_Type_create_struct (count,
			       array_of_blocklengths,
			       array_of_displacements,
			       array_of_types,
			       newtype);
  assert(rc==MPI_SUCCESS);
  MPI_Datatype temp_packed_type, packed_type;
  int array_of_p_blocklengths[count];
  MPI_Aint array_of_p_displacements[count];
  MPI_Datatype array_of_p_types[count], datatype;
  int s=0, is_active, is_derived, dt_idx;
  MPI_Aint p_mapsize=0, extent, lb;
  derivedTypeData* dat = getDTypeData();
  for (i=0;i<count;i++) {
    datatype = array_of_types[i];
    is_active = (*ourADTOOL_AMPI_FPCollection.isActiveType_fp)(datatype)==AMPI_ACTIVE;
    dt_idx = derivedTypeIdx(datatype);
    is_derived = isDerivedType(dt_idx);
    array_of_p_blocklengths[i] = array_of_blocklengths[i];
    array_of_p_displacements[i] = p_mapsize;
    if (is_active) {
      array_of_p_types[i] = MPI_DOUBLE;
      s = sizeof(double);
    }
    else if (is_derived) {
      array_of_p_types[i] = dat->packed_types[dt_idx];
      s = dat->p_extents[dt_idx];
    }
    else {
      array_of_p_types[i] = array_of_types[i];
      if (array_of_types[i]==MPI_DOUBLE) s = sizeof(double);
      else if (array_of_types[i]==MPI_INT) s = sizeof(int);
      else if (array_of_types[i]==MPI_FLOAT) s = sizeof(float);
      else if (array_of_types[i]==MPI_CHAR) s = sizeof(char);
      else assert(0);
    }
    p_mapsize += array_of_blocklengths[i]*s;
  }
  MPI_Type_get_extent(*newtype,&lb,&extent);
  rc = MPI_Type_create_struct (count,
			       array_of_p_blocklengths,
			       array_of_p_displacements,
			       array_of_p_types,
			       &temp_packed_type);
  assert(rc==MPI_SUCCESS);
  rc = MPI_Type_create_resized (temp_packed_type,
				0,
				(MPI_Aint)p_mapsize,
				&packed_type);
  addDTypeData(dat,
	       count,
	       array_of_blocklengths,
	       array_of_displacements,
	       array_of_types,
	       lb,
	       extent,
	       array_of_p_blocklengths,
	       array_of_p_displacements,
	       array_of_p_types,
	       p_mapsize,
	       newtype,
	       &packed_type);
  MPI_Type_free(&temp_packed_type);
  return rc;
}

int AMPI_Type_commit_NT(MPI_Datatype *datatype) {
  int dt_idx = derivedTypeIdx(*datatype);
  if (isDerivedType(dt_idx)) MPI_Type_commit(&(getDTypeData()->packed_types[dt_idx]));
  return MPI_Type_commit(datatype);
}

int AMPI_Type_create_resized_NT(MPI_Datatype oldtype,
				MPI_Aint lb,
				MPI_Aint extent,
				MPI_Datatype *newtype) {
  int rc;
  rc = MPI_Type_create_resized(oldtype,
			       lb,
			       extent,
			       newtype);
  int dt_idx = derivedTypeIdx(oldtype);
  if (isDerivedType(dt_idx)) {
    derivedTypeData* dtd = getDTypeData();
    dtd->lbs[dt_idx] = lb;
    dtd->extents[dt_idx] = extent;
    dtd->derived_types[dt_idx] = *newtype;
  }
  return rc;
}

int AMPI_Op_create_NT(MPI_User_function *function,
		      int commute,
		      MPI_Op *op) {
  int rc;
  rc = MPI_Op_create(function,
		     commute,
		     op);
  if (!(rc==MPI_SUCCESS)) assert(0);
  userDefinedOpData* dat = getUOpData();
  addUOpData(dat,
	     op,
	     function,
	     commute);
  return rc;
}

int AMPI_Type_free_NT(MPI_Datatype *datatype) {
  int dt_idx = derivedTypeIdx(*datatype);
  if (isDerivedType(dt_idx)) MPI_Type_free(&(getDTypeData()->packed_types[dt_idx]));
  return MPI_Type_free(datatype);
}

int AMPI_Op_free_NT(MPI_Op *op) {
  return MPI_Op_free(op);
}
