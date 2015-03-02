/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
/* Generic AMPI C tape. Don't touch this. The code is always mirrored with the AMPI repo.
 * Changing code here will result in merge conflicts.
 *
 * See header for more information. 
 */
/*#define DEBUG*/
#define NO_COMM_WORLD
#include <ampi_tape.h> 


ampi_tape_entry *ampi_tape;
/*int ampi_tape_counter = 0;*/
int ampi_vac=0;
int ampi_chunks=1;

int AMPI_Reset_Tape() {
    ampi_vac=0;
    printf("AMPI tape has been reset.\n");
    return 0;
}

int AMPI_Init(int* argc, char*** argv) {
    int size=AMPI_CHUNK_SIZE;
    ampi_tape = malloc(size*sizeof(ampi_tape_entry));
    return AMPI_Init_f(argc, argv);
}

int AMPI_Finalize() {
    printf("AMPI chunk size: %d\n", AMPI_CHUNK_SIZE);
    printf("AMPI chunks allocated: %d\n", ampi_chunks);
    printf("AMPI memory allocated: %lu\n", ampi_chunks*AMPI_CHUNK_SIZE*sizeof(ampi_tape_entry));
    return AMPI_Init_b(NULL, NULL);

}

int AMPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request) {
    int i=0;
    request->buf = buf;
    double * tmp = malloc(sizeof(double)*count);
    ampi_tape[ampi_vac+count].arg=malloc(sizeof(int)*2);
    ampi_check_tape_size(count+1);
    int new_vac = ampi_vac+count;

    ampi_create_tape_entry(&new_vac);

    /*create dummy of each element*/

#pragma omp parallel for
    for(i=0 ; i<count ; i=i+1) {
	ampi_tape[ampi_vac+i].oc = MPI_DUMMY;
	ampi_get_val(buf,&i,&tmp[i]);
	ampi_get_idx(buf, &i, &ampi_tape[ampi_vac+i].idx);
    }
    ampi_tape[ampi_vac+count].oc = ISEND;
    ampi_tape[ampi_vac+count].arg[0] = count;
    ampi_tape[ampi_vac+count].arg[1] = dest;
    ampi_tape[ampi_vac+count].comm = comm;
    ampi_tape[ampi_vac+count].tag = tag;

    /*if(request->aw) { */
	/*if antiwait, memory was already allocated*/
	/*tape[tape_entry::vac+count].request = new AMPI_dco_Request;*/
	/*tape[request->va].request = tape[tape_entry::vac+count].request;*/
	/*tape[request->va].arg[0] = tape_entry::vac+count;*/
    /*}*/
    /*else {*/
	ampi_tape[ampi_vac+count].request = malloc(sizeof(AMPI_Request));
	/*tape[tape_entry::vac+count].request = new AMPI_dco_Request;*/
	/*}*/
    /*point current request index to this tape entry*/
    request->va = ampi_vac+count; 
    request->v = tmp;
    int temp = AMPI_Isend_f(tmp, count, datatype, dest, tag, comm, request);
    ampi_vac+=count+1;
    return temp;
}

int AMPI_Irecv(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, AMPI_Request *request) {
    int i=0;
    /*INT64 tmp_int64 = 0;*/
    request->buf = buf;
    double * tmp = malloc(sizeof(double)*count);
    ampi_tape[ampi_vac].arg=malloc(sizeof(int)*2);
    ampi_check_tape_size(count+1);
    int new_vac = ampi_vac;

    ampi_create_dummies(buf, &count);
    ampi_create_tape_entry(&new_vac);

    int temp = AMPI_Irecv_f(tmp, count, datatype, dest, tag, comm, request);

    ampi_tape[ampi_vac].arg[0] = count;
    ampi_tape[ampi_vac].arg[1] = dest;
    ampi_tape[ampi_vac].oc = IRECV;
    ampi_tape[ampi_vac].comm = comm;
    ampi_tape[ampi_vac].tag = tag;

#pragma omp parallel for
    for(i=0 ; i<count ; i=i+1) {
	/*ampi_get_idx(buf, &i, &tmp_int64);*/
	ampi_get_idx(buf, &i, &ampi_tape[ampi_vac+i+1].idx);
	/*ampi_tape[ampi_vac+i+1].idx = tmp_int64;*/
	ampi_tape[ampi_vac+i+1].oc = MPI_DUMMY;


    } 
    /*if(request->aw) { */
	/*if antiwait, memory was already allocated*/
	/*tape[tape_entry::vac].request = new AMPI_dco_Request;*/
	/*tape[request->va].request = tape[tape_entry::vac].request;*/
	/*tape[request->va].arg[0] = tape_entry::vac;*/
    /*}*/
    /*else {*/
	ampi_tape[ampi_vac].request = malloc(sizeof(AMPI_Request));
	/*tape[tape_entry::vac].request =  new AMPI_dco_Request;*/
	/*}*/
    /*request->r.v = tmp;*/
    /*point current request index to this tape entry*/
    request->va = ampi_vac; 
    ampi_vac+=count+1;
    return temp;
}

int AMPI_Wait(AMPI_Request *request, MPI_Status *status) {
    int i=0;
    double * tmp = (double*) request->v;
    ampi_tape[ampi_vac].arg=malloc(sizeof(int));
    ampi_check_tape_size(1);
    int new_vac = ampi_vac;

    ampi_create_tape_entry(&new_vac);
    /*get the corresponding isend or irecv tape entry*/

    ampi_tape[ampi_vac].arg[0] = request->va;    
    ampi_tape[ampi_vac].oc = WAIT;
    AMPI_Wait_f(request, status);
    /*finally copy the request to the tape*/
    request->tag=status->MPI_TAG;
    *ampi_tape[request->va].request = *request;			
    ampi_tape[ampi_vac].request = ampi_tape[request->va].request;
    if(request->oc == AMPI_IR) {
	for(i=0 ; i<ampi_tape[request->va].arg[0] ; i=i+1) {
	    ampi_set_val(request->buf, &i, &tmp[i]);
	}	 

    }
    /*ampi_tape[ampi_vac].request->a = &ampi_tape[request->va].d;*/
    ampi_tape[ampi_vac].request->size = ampi_tape[request->va].arg[0];
    ampi_vac++;
    free(tmp);
    return 0;
}

void ampi_interpret_tape(){ 
    int j=0;
    double *tmp_d;
    double *tmp_d_recv;
    double *tmp_d_send;
    int i=ampi_vac;
    ampi_tape_entry *tmp_entry;
    MPI_Comm comm;
    MPI_Op op = MPI_SUM;
    comm = MPI_COMM_WORLD;
    MPI_Status status;
    ampi_vac=ampi_vac-1;
    while(ampi_tape[ampi_vac].oc == MPI_DUMMY)
	ampi_vac=ampi_vac-1;
    i=ampi_vac;
#ifdef DEBUG
    printf("AMPI_TAPE Interpreter OC: %d\n", ampi_tape[ampi_vac].oc);
#endif
    switch(ampi_tape[ampi_vac].oc){ 
	case ISEND : {
			 /*tmp_d = malloc(sizeof(double)*ampi_tape[i].arg[0]);*/
			 /*if(!tape[i].request->r.aw) {*/
			 tmp_d = (double*) ampi_tape[i].request->a;
#ifdef NO_COMM_WORLD
			comm=ampi_tape[i].comm;
#endif
			 AMPI_Isend_b(tmp_d, ampi_tape[i].arg[0], MPI_DOUBLE, ampi_tape[i].arg[1], ampi_tape[i].tag, comm,ampi_tape[i].request);
			 for(j = 0 ; j < ampi_tape[i].arg[0] ; j++) {
			     tmp_entry=&ampi_tape[i-ampi_tape[i].arg[0]+j];
			     ampi_set_adj(&tmp_entry->idx, &tmp_d[j]);
			     /*ampi_set_adj(&ampi_tape[i-j-1].idx, &tmp_d[j]);*/
			     /*}*/
		     }
		     /*else {*/
		     /*}*/
			 free(tmp_d);
			 break;
		     }
	case IRECV : {
			 /*tmp_d = malloc(sizeof(double)*ampi_tape[i].arg[0]);*/
			 tmp_d = (double*) ampi_tape[i].request->a;
#ifdef NO_COMM_WORLD
			comm=ampi_tape[i].comm;
#endif
			 /*if(tape[i].request->r.aw) {*/

			 /*}*/
			 /*else {*/
			     AMPI_Irecv_b(tmp_d, ampi_tape[i].arg[0], MPI_DOUBLE, ampi_tape[i].arg[1], ampi_tape[i].tag, comm,ampi_tape[i].request);
			     /*}*/
			 free(tmp_d);
			 break;
		     }
	case WAIT : {
			tmp_d = malloc(sizeof(double)*ampi_tape[ampi_tape[i].arg[0]].arg[0]);
			if(ampi_tape[i].request->oc == AMPI_IR) {
			    for(j = 0 ; j < ampi_tape[ampi_tape[i].arg[0]].arg[0] ; j++) {
				tmp_entry=&ampi_tape[ampi_tape[i].arg[0]+j+1];
				ampi_get_adj(&tmp_entry->idx, &tmp_d[j]);
			    }
#ifdef DEBUG
			    printf("AMPI_Wait_interpret: ");
			    printf("%d ", ampi_tape[ampi_tape[i].arg[0]].arg[0]);
			    for(j = 0 ; j < ampi_tape[ampi_tape[i].arg[0]].arg[0] ; j++) {
				printf("%e ", tmp_d[j]);
			    }
			    printf("\n");
#endif
			}
			ampi_tape[i].request->a = tmp_d;
#ifndef NO_COMM_WORLD 
			ampi_tape[i].request->comm=MPI_COMM_WORLD;
#endif
			AMPI_Wait_b(ampi_tape[i].request,&status);
			ampi_tape[ampi_tape[i].arg[0]].request = ampi_tape[i].request;
			break;
		    }
	default: {
		     printf("Error: Missing opcode in the AMPI tape interpreter for %d at tape index %d.\n", ampi_tape[ampi_vac].oc, ampi_vac);
		     break;
		      }

    }
}
void ampi_print_tape() {
    /* TODO error in printout */
    /*int i=0;*/
    /*for(i=ampi_vac;i>0;i--) {*/
    /*printf("IDX: %d, OC: %d, arg[0]: %d, arg[1]: %d\n", i-1,ampi_tape[i-1].oc, ampi_tape[i-1].arg[0], ampi_tape[i-1].arg[1]);*/
    /*}*/
}
void ampi_print_tape_entry(int *i) {
    /*printf("  -----------------------------------------\n");*/
    printf("             AMPI CALL: ");
    switch(ampi_tape[*i].oc){
	case SEND : {
			printf("SEND");
			break;
		    }
	case RECV : {
			printf("RECV");
			break;
		    }
	case IRECV : {
			printf("IRECV");
			break;
		    }
	case ISEND : {
			printf("ISEND");
			break;
		    }
	case WAIT : {
			printf("WAIT");
			break;
		    }
	case REDUCE : {
			printf("REDUCE");
			break;
		    }
	case ALLREDUCE : {
			printf("ALLREDUCE");
			break;
		    }
    }
    printf("\n");
    /*printf("  -----------------------------------------\n");*/
}

void ampi_check_tape_size(int size) {
    if(ampi_vac>=ampi_chunks*AMPI_CHUNK_SIZE-size) {
	ampi_tape_entry *tmp;
        tmp=realloc(ampi_tape,(ampi_chunks+1)*AMPI_CHUNK_SIZE*sizeof(ampi_tape_entry));
	if(tmp != NULL) {
	    ampi_tape=tmp;
	    ampi_chunks=ampi_chunks+1;
	}
	else {
	    printf("AMPI tape allocation error.\n");
	}
    }
}
