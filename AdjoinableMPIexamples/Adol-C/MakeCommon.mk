ifndef AMPIROOT
 $(error "require AMPIROOT environment variable")
endif

ifndef MPICC
MPICC=mpicc
endif

ifndef MPICXX
MPICXX=mpicxx
endif

ifndef MPIRUN
MPIRUN=mpirun
endif

CXXFLAGS=-g -O0

default: driver.out fdDriver.out ampiDriver.out
	cat $^
	../../diffTest.py $^

driver.out : driver
	$(MPIRUN) -n $(NPROC)  ./$< > $@

fdDriver.out : fdDriver
	$(MPIRUN) -n $(NPROC)  ./$< > $@

ampiDriver.out : ampiDriver
	$(MPIRUN) -n $(NPROC) ./$< > $@
#	$(MPIRUN) -n $(NPROC) valgrind --track-origins=yes --gen-suppressions=all --suppressions=../vgSupp.txt -q --log-file=vg_%p.txt --leak-check=full ./$< > $@

debug: ampiDriver
	$(MPIRUN) -n $(NPROC) konsole --profile gdb --workdir . -e gdb -x ./gdbCmd ./$< 

cleanDebug: clean
	-ps -ef | grep ampiDriver | awk '{print $$2}' | xargs kill -9 

driver: driver.c
	${MPICC} $(CXXFLAGS) -o $@ $^ -lm

fdDriver: fdDriver.c
	${MPICC} $(CXXFLAGS) -o $@ $^ -lm

ampiDriver.o: ampiDriver.cpp
	$(MPICXX) $(CXXFLAGS) -DTYPE_MACRO=AMPI_ADOUBLE -I$(AMPIROOT)/include -I$(ADOLC_DIR)/include -c -o $@ $^

ampiDriver: ampiDriver.o
	$(MPICXX) $(CXXFLAGS) -o $@ $^ -L$(ADOLC_DIR)/lib -ladolc -L$(AMPIROOT)/lib -lampiCommon -lampiBookkeeping -lampiTape

clean: 
	rm -f *.out driver fdDriver ampiDriver *.o tape_* ADOLC-*.tap vg*.txt
