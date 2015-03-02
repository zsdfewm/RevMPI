##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
lib_LTLIBRARIES = libampiPlainC.la

libampiPlainC_la_CPPFLAGS = -I@top_builddir@ -I@abs_top_srcdir@

libampiPlainC_la_SOURCES = \
modified.c \
nt.c \
passThrough.c \
st.c

if ISFORTRANCOMPATIBLE
libampiPlainC_la_SOURCES += ampi_f2c.c
endif
