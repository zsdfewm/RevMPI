##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
userincludedir = @includedir@/ampi
userinclude_HEADERS = \
ampi.h 

if ISFORTRANCOMPATIBLE
userinclude_HEADERS += ampif.h
endif

userIFincludedir = @includedir@/ampi/userIF
userIFinclude_HEADERS = \
userIF/activity.h \
userIF/libConfig.h \
userIF/modified.h \
userIF/nt.h \
userIF/pairedWith.h \
userIF/passThrough.h \
userIF/request.h \
userIF/window.h \
userIF/st.h

adToolincludedir = @includedir@/ampi/adTool
adToolinclude_HEADERS = \
adTool/support.h 

tapeincludedir = @includedir@/ampi/tape
tapeinclude_HEADERS = \
tape/support.h

libCommonincludedir = @includedir@/ampi/libCommon
libCommoninclude_HEADERS = \
libCommon/modified.h \
libCommon/st.h 
