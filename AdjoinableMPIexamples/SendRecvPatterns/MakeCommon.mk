ifndef AMPIROOT
 $(error "require AMPIROOT environment variable")
endif

ifndef MPICC
MPICC=mpicc
endif

default: driver.out ampiDriver.out
	cat $^

driver.out : driver
	mpirun -n $(NPROC)  ./$< > $@

ampiDriver.out : ampiDriver
	mpirun -n $(NPROC) ./$< > $@

driver: driver.c
	${MPICC} -o $@ $^

ampiDriver: ampiDriver.c
	$(MPICC) -DTYPE_MACRO=MPI_DOUBLE -I$(AMPIROOT)/include -o $@ $^ -L$(AMPIROOT)/lib -lampiPlainC

clean: 
	rm -f *.out driver ampiDriver 
