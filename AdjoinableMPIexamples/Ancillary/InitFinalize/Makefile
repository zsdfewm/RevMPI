ifndef AMPIROOT
 $(error "require AMPIROOT environment variable")
endif

ifndef MPICC
MPICC=mpicc
endif

default: driver.out ampiDriver.out
	cat $^

driver.out : driver FORCE
	mpirun -n 3 ./$< > $@

ampiDriver.out : ampiDriver FORCE
	mpirun -n 3 ./$< > $@

driver: driver.c
	${MPICC} -o $@ $^

ampiDriver: ampiDriver.c
	$(MPICC) -I$(AMPIROOT)/include -o $@ $^ -L$(AMPIROOT)/lib -lampiPlainC

FORCE: 

clean: 
	rm -f *.out driver ampiDriver 

.PHONY: clean FORCE default
