##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
AC_DEFUN([MPI_CONF],
[
AC_PREREQ(2.59)

# MPI root directory
AC_ARG_WITH(mpi_root,
[AC_HELP_STRING([--with-mpi-root=MPIROOT],
		[absolute path to the MPI root directory])])

if test x"$with_mpi_root" != "x"; 
then 
  MPIROOT="$with_mpi_root"
fi

AC_ARG_WITH(mpif77,
[AC_HELP_STRING([--with-mpif77=MPIF77],
                [name of the MPI Fortran77 compiler to use; needed only for --enable-fortranCompatible ; (default mpif77)])])

if test "x$fortranCompatible" = "x#define AMPI_FORTRANCOMPATIBLE"; 
then 
  if test x"$with_mpif77" != "x"; 
  then 
    MPIF77="$with_mpif77"
  else 
    MPIF77="mpif77"
  fi

  if test x"$with_mpi_root" != "x"; 
  then 
    MPIF77="$with_mpi_root/bin/$MPIF77"
  fi
  F77=$MPIF77
fi

AC_ARG_WITH(mpicc,
[AC_HELP_STRING([--with-mpicc=MPICC],
		[name of the MPI C++ compiler to use (default mpicc)])])

if test x"$with_mpicc" != "x"; 
then 
  MPICC="$with_mpicc"
else 
  MPICC="mpicc"
fi

if test x"$with_mpi_root" != "x"; 
then 
  MPICC="$with_mpi_root/bin/$MPICC"
fi

# from here on everything goes with the MPI C compiler
CPP=$MPICC
CC=$MPICC
LD=$MPICC

AC_MSG_CHECKING([MPI C compiler])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <mpi.h>],
                              	[MPI_Init(0,0)])],
               [AC_MSG_RESULT([ok])],
               [AC_MSG_RESULT([no])
               AC_MSG_FAILURE([MPI C compiler is required by $PACKAGE])])


]) 

