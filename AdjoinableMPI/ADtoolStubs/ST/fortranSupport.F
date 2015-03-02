!##########################################################
!# This file is part of the AdjoinableMPI library         #
!# released under the MIT License.                        #
!# The full COPYRIGHT notice can be found in the top      #
!# level directory of the AdjoinableMPI distribution.     #
!########################################################## 
      subroutine ADTOOL_AMPI_fortranSetupTypes(adouble, areal)
#include "ampi/ampif.h"
        INTEGER adouble
        INTEGER areal
C for now just set this to be scalars of the matching type 
        AMPI_ADOUBLE_PRECISION=MPI_DOUBLE_PRECISION
        adouble=AMPI_ADOUBLE_PRECISION
        AMPI_AREAL=MPI_REAL
        areal=AMPI_AREAL
      end subroutine

      subroutine ADTOOL_AMPI_fortranCleanupTypes(adouble, areal)
#include "ampi/ampif.h"
        INTEGER adouble
        INTEGER areal
C for now do nothing
      end subroutine
