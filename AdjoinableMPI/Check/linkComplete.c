/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include "ampi/ampi.h"

int main(void) { 
 AMPI_Init_NT(0,0);
 AMPI_Finalize_NT();
 return 0; 
}
