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

default: driver.out fdDriver.out ampiDriver.out ampiTLMdriver.out
	cat $^
	../../diffTest.py $^

driver.out : driver
	$(MPIRUN) -n $(NPROC)  ./$< > $@

fdDriver.out : fdDriver
	$(MPIRUN) -n $(NPROC)  ./$< > $@

ampiDriver.out : ampiDriver
	$(MPIRUN) -n $(NPROC) ./$< > $@

ampiTLMdriver.out : ampiTLMdriver
	$(MPIRUN) -n $(NPROC) ./$< > $@
#	$(MPIRUN) -n $(NPROC) valgrind --track-origins=yes --gen-suppressions=all --suppressions=../vgSupp.txt -q --log-file=vg_%p.txt --leak-check=no ./$< > $@

debug: ampiTLMdriver
	$(MPIRUN) -n $(NPROC) konsole --profile gdb --workdir . -e gdb -x ./gdbCmd ./$< 

head.o: head.c head.h
	${MPICC} $(CXXFLAGS) -o $@ -c $<

driver.o: driver.c head.h
	${MPICC} $(CXXFLAGS) -o $@ -c $<

driver : driver.o head.o
	${MPICC} -o $@ $^  -lm

fdDriver.o: fdDriver.c head.h
	${MPICC} $(CXXFLAGS) -o $@ -c $< -lm

fdDriver : fdDriver.o head.o
	${MPICC} -o $@ $^  -lm

ampiDriver.o: ampiDriver.cpp ampiHead.h
	$(MPICXX) $(CXXFLAGS) -DTYPE_MACRO=AMPI_ADOUBLE -I$(AMPIROOT)/include -I$(ADOLC_DIR)/include -o $@ -c $<

ampiHead.o: ampiHead.cpp  ampiHead.h
	$(MPICXX) $(CXXFLAGS) -DTYPE_MACRO=AMPI_ADOUBLE -I$(AMPIROOT)/include -I$(ADOLC_DIR)/include -o $@ -c $<

ampiDriver: ampiDriver.o ampiHead.o
	$(MPICXX) $(CXXFLAGS) -o $@ $^ -L$(ADOLC_DIR)/lib -ladolc -L$(AMPIROOT)/lib -lampiCommon -lampiBookkeeping -lampiTape

ampiTLMdriver.o: ampiTLMdriver.cpp ampiHead.h
	$(MPICXX) $(CXXFLAGS) -DTYPE_MACRO=AMPI_ADOUBLE -I$(AMPIROOT)/include -I$(ADOLC_DIR)/include -o $@ -c $<

ampiTLMdriver: ampiTLMdriver.o ampiHead.o
	$(MPICXX) $(CXXFLAGS) -o $@ $^ -L$(ADOLC_DIR)/lib -ladolc -L$(AMPIROOT)/lib -lampiCommon -lampiBookkeeping -lampiTape

ampiHead.cpp: ampiHead.h
ampiDriver.cpp: ampiHead.h
ampiTLMdriver.cpp: ampiHead.h

debugClean: clean
	-ps -ef | grep ampiTLMdriver | awk '{print $$2}' | xargs kill -9 

clean: 
	rm -f *.out driver fdDriver ampiDriver ampiTLMdriver *.o tape_* ADOLC-*.tap vg*.txt
