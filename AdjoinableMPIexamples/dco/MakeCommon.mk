ifndef AMPIROOT
 $(error "require AMPIROOT environment variable")
endif

ifndef MPICC
MPICC=mpicc
endif

ifndef MPICXX
MPICXX=mpicxx
endif

DCO_BUILD=$(DCO_BASE_DIR)/build/dco_cpp

CXXFLAGS=-g -O0

default: driver.out fdDriver.out ampiDriver.out
#default: ampiDriver.out
	cat $^

driver.out : driver
	mpirun -n $(NPROC)  ./$< > $@

fdDriver.out : fdDriver
	mpirun -n $(NPROC)  ./$< > $@

ampiDriver.out : ampiDriver
	mpirun -n $(NPROC) ./$< > $@

debug: ampiDriver
	mpirun -n $(NPROC) konsole --profile gdb --workdir . -e gdb -x ./gdbCmd ./$< 

driver: driver.c
	${MPICC} $(CXXFLAGS) -o $@ $^ -lm

fdDriver: fdDriver.c
	${MPICC} $(CXXFLAGS) -o $@ $^ -lm

ampiDriver.o: ampiDriver.cpp
	$(MPICXX) $(CXXFLAGS) -DTYPE_MACRO=AMPI_ADOUBLE -DACTIVITY_MACRO=AMPI_ACTIVE -I$(AMPIROOT)/include -I$(DCO_BUILD)/include -c -o $@ $^

ampisupport.o: ampisupport.cpp
	$(MPICXX) $(CXXFLAGS) -DTYPE_MACRO=AMPI_ADOUBLE -DACTIVITY_MACRO=AMPI_ACTIVE -I$(AMPIROOT)/include -I$(DCO_BUILD)/include -c -o $@ $^

#ampi_interface.o: ampi_interface.cc
	#$(MPICXX) $(CXXFLAGS) -DACTIVITY_MACRO=AMPI_ACTIVE -I$(AMPIROOT)/include -I$(DCO_BUILD) -c -o $@ $^

ampiDriver: ampiDriver.o ampisupport.o
	$(MPICXX) $(CXXFLAGS) -o $@ $^ $(AMPIROOT)/lib/libampiCommon.a $(DCO_BUILD)/lib/libdco.a $(AMPIROOT)/lib/libampiCommon.a $(AMPIROOT)/lib/libampiBookkeeping.a $(AMPIROOT)/lib/libampiTape.a
#	$(MPICXX) $(CXXFLAGS) -o $@ $^ -L$(AMPIROOT)/lib -lampiCommon -L$(ADOLC_DIR)/lib -ladolc -lampiCommon

  
check: ampiDriver
	mpiexec -n $(NPROC) ./$< > output
	diff output all_ok.check

clean: 
	#-ps -ef | grep ampiDriver | awk '{print $$2}' | xargs kill -9 
	rm -f *.out driver fdDriver ampiDriver *.o tape_* ADOLC-*.tap
