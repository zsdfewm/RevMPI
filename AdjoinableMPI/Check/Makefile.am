##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
check_PROGRAMS         = linkCompleteOO linkCompleteST

linkCompleteOO_SOURCES = linkComplete.c
linkCompleteST_SOURCES = linkComplete.c

AM_CPPFLAGS            = -I@top_builddir@ -I@abs_top_srcdir@

linkCompleteST_LDADD   = ../Common/libampiCommon.la ../Bookkeeping/libampiBookkeeping.la ../ADtoolStubs/ST/libampiADtoolStubsST.la
linkCompleteOO_LDADD   = ../Common/libampiCommon.la ../Bookkeeping/libampiBookkeeping.la ../ADtoolStubs/OO/libampiADtoolStubsOO.la

TESTS = $(check_PROGRAMS)
