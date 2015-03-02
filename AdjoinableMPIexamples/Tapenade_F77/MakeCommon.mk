ifndef AMPIROOT
 $(error "require AMPIROOT environment variable")
endif

ifndef TAPENADE_HOME
 $(error "require TAPENADE_HOME environment variable")
endif

ifndef MPICC
MPICC=mpicc
endif

ifndef MPIF77
MPIF77=mpif77
endif

ifndef MPIRUN
MPIRUN=mpirun
endif

ifndef NPROC
NPROC=2
endif

FFLAGS=-g -O0
CFLAGS=-g -O0

default : ref.out orig.out tgt.out adj.out
	cat ref.out orig.out tgt.out adj.out

clean : 
	-ps -ef | grep adj | awk '{print $$2}' | xargs kill -9 
	rm -f *.o *.out ref orig tgt adj

ref.out : ref
	$(MPIRUN) -np $(NPROC)  ./$< > $@ 2>&1

orig.out : orig
	$(MPIRUN) -np $(NPROC)  ./$< > $@ 2>&1

tgt.out : tgt
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(AMPIROOT)/lib/ ; $(MPIRUN) -np $(NPROC)  ./$< > $@ 2>&1

adj.out : adj
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(AMPIROOT)/lib/ ; $(MPIRUN) -np $(NPROC)  ./$< > $@ 2>&1

ref : program.f
	$(MPIF77) $(FFLAGS) -o $@ $^ -lm

orig : programAMPI.f fortranSupport.o
	$(MPIF77) $(FFLAGS) -I$(AMPIROOT)/include -o $@ $^ -L$(AMPIROOT)/lib -lampiPlainC -lm

tgt : programAMPI_d.f ampiSupport.o fortranSupport.o $(TAPENADE_HOME)/ADFirstAidKit/adStack.c
	$(MPIF77) $(FFLAGS) -I$(TAPENADE_HOME) -I$(AMPIROOT)/include $^ -L$(AMPIROOT)/lib -lampiCommon -lampiBookkeeping -o $@

adj : programAMPI_b.f ampiSupport.o fortranSupport.o $(TAPENADE_HOME)/ADFirstAidKit/adStack.c $(TAPENADE_HOME)/ADFirstAidKit/adBuffer.f
	$(MPIF77) $(FFLAGS) -I$(TAPENADE_HOME) -I$(AMPIROOT)/include $^ -L$(AMPIROOT)/lib -lampiCommon -lampiBookkeeping -o $@



fortranSupport.o : ../fortranSupport.F
	$(MPIF77) $(FFLAGS) -I$(TAPENADE_HOME) -I$(AMPIROOT)/include -c $^ -o $@

ampiSupport.o : ../ampiSupport.c
	$(MPICC) $(CFLAGS) -I$(TAPENADE_HOME) -I$(AMPIROOT)/include -c $^ -o $@

debugadj: adj
	$(MPIRUN) -n $(NPROC)  konsole --profile gdb --workdir . -e gdb -x ./gdbCmd ./$< 

.PHONY: clean debug default
