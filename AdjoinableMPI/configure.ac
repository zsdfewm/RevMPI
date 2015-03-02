dnl ##########################################################
dnl # This file is part of the AdjoinableMPI library         #
dnl # released under the MIT License.                        #
dnl # The full COPYRIGHT notice can be found in the top      #
dnl # level directory of the AdjoinableMPI distribution.     #
dnl ########################################################## 
dnl Process this file with autoconf to produce a configure script.

dnl Disable caching: 
define([AC_CACHE_LOAD], )dnl
define([AC_CACHE_SAVE], )dnl

AC_PREREQ(2.61)
AC_INIT([AMPI], [0.1])
dnl macros in config/*.m4
AC_CONFIG_MACRO_DIR([config])
dnl helper tools are in config
AC_CONFIG_AUX_DIR([config])
dnl check if at least one source file is there ...
AC_CONFIG_SRCDIR([ampi/ampi.h])
AC_CANONICAL_TARGET

dnl ---- Automake ----
dnl foreign   -> do no create the standard GNU standard files NEWS README AUTHORS ChangeLog
dnl no-define -> do not define PACKAGE and VERSION
AM_INIT_AUTOMAKE([foreign no-define])
dnl Do not show compilation command by default
AM_SILENT_RULES([yes])

AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_SUBST(fortranCompatible)
AC_ARG_ENABLE(fortranCompatible,
[AS_HELP_STRING([--enable-fortranCompatible],
                [adjust the signatures to be callable from Fortran])],
[case "${enableval}" in
  yes)
    fortranCompatible='#define AMPI_FORTRANCOMPATIBLE'
    ;;
  no)
    fortranCompatible='#undef AMPI_FORTRANCOMPATIBLE'
    ;;
  *)
     AC_MSG_ERROR(bad value ${fortranCompatible} for --enable-fortranCompatible) ;;
esac],
[fortranCompatible='#undef AMPI_FORTRANCOMPATIBLE'])

AM_CONDITIONAL(ISFORTRANCOMPATIBLE, [ test "x$fortranCompatible" = "x#define AMPI_FORTRANCOMPATIBLE" ] )

if test "x$fortranCompatible" = "x#define AMPI_FORTRANCOMPATIBLE"; 
then 
AC_PROG_F77
fi

AC_SUBST(requestOnTrace)
AC_ARG_ENABLE(requestOnTrace,
[AS_HELP_STRING([--enable-requestOnTrace],
                [trace reverse sweep MPI request by forward sweep in the internal bookkeeping (typically not needed for source transformation AD)])],
[case "${enableval}" in
  yes)
    requestOnTrace='#define AMPI_REQUESTONTRACE'
    ;;
  no)
    requestOnTrace='#undef AMPI_REQUESTONTRACE'
    ;;
  *)
     AC_MSG_ERROR(bad value ${requestOnTrace} for --enable-requestOnTrace) ;;
esac],
[requestOnTrace='#undef AMPI_REQUESTONTRACE'])

AC_LANG([C])
MPI_CONF
dnl try to keep things clean: 
CFLAGS="-Wall -Wno-unknown-pragmas -std=c89 "${CFLAGS}

AC_CONFIG_FILES([Makefile
		 ampi/Makefile
		 ampi/userIF/libConfig.h
                 PlainC/Makefile
		 Bookkeeping/Makefile
		 Tape/Makefile
		 Common/Makefile
		 ADtoolStubs/ST/Makefile
		 ADtoolStubs/OO/Makefile
		 Check/Makefile])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_OUTPUT
