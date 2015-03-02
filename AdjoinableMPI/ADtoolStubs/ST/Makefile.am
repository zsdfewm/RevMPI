##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
lib_LTLIBRARIES = libampiADtoolStubsST.la

libampiADtoolStubsST_la_CPPFLAGS = -I@top_builddir@ -I@abs_top_srcdir@

libampiADtoolStubsST_la_SOURCES = \
support.c 

## Automake attempts to guess the correct linker among the various compilers
## for each language (see "How the Linker is Chosen" in the AM manual).
## However, this process is static and doesn't assume that you will "disable"
## Fortran support for a library and still actually build that library.
## lib@MPILIBNAME@.la contains both C and F77 source, so AM picks "F77LD" as the
## linker.  Instead we manually override automake's choice based on the value of
## enable_f77.
if ISFORTRANCOMPATIBLE
libampiADtoolStubsST_la_SOURCES += fortranSupport.F
# link with libtool+F77LD
libampiADtoolStubsST_la_LINK = $(LIBTOOL) $(AM_V_lt) --tag=F77 \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link $(F77LD) \
	$(AM_FFLAGS) $(FFLAGS) $(libampiADtoolStubsST_la_LDFLAGS) \
	$(LDFLAGS) -o $@
else !ISFORTRANCOMPATIBLE
# link with libtool+CCLD
libampiADtoolStubsST_la_LINK = $(LIBTOOL) $(AM_V_lt) --tag=CC \
	$(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=link $(CCLD) \
	$(libampiADtoolStubsST_la_CFLAGS) $(CFLAGS) \
	$(libampiADtoolStubsST_la_LDFLAGS) $(LDFLAGS) -o $@
endif !ISFORTRANCOMPATIBLE
