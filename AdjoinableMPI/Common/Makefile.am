##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
lib_LTLIBRARIES = libampiCommon.la

libampiCommon_la_CPPFLAGS = -I@top_builddir@ -I@abs_top_srcdir@

libampiCommon_la_SOURCES = \
modified.c \
nt.c \
passThrough.c \
window.c \
st.c

if ISFORTRANCOMPATIBLE
libampiCommon_la_SOURCES += ampi_f2c.c
endif
