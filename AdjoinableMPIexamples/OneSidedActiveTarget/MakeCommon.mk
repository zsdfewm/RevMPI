ifndef AMPIROOT
 $(error "require AMPIROOT environment variable")
endif

ifndef MPICC
MPICC=mpicc
endif

default: driver.out 
	cat $^

driver.out : driver
	mpirun -n $(NPROC)  ./$< > $@

ampiDriver.out : ampiDriver
	mpirun -n $(NPROC) ./$< > $@

driver: driver.c
	${MPICC} -g -O0 -o $@ $^

debug: driver
	mpirun -n $(NPROC) konsole --profile gdb --workdir . -e gdb -x ./gdbCmd ./$< 

ampiDriver: ampiDriver.c
	$(MPICC) -DTYPE_MACRO=MPI_DOUBLE -I$(AMPIROOT)/include -o $@ $^ -L$(AMPIROOT)/lib -lampiPlainC

clean: 
	-ps -ef | grep driver | awk '{print $$2}' | xargs kill -9 
	rm -f *.out driver ampiDriver 

.PHONY: clean default debug
