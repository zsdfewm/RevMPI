/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include "ampi/libCommon/modified.h"
#include "ampi/bookkeeping/support.h"
#include "ampi/adTool/support.h"


void AMPI_WIN_STACK_push(AMPI_Win_stack *s, AMPI_WinRequest val) {
    if(AMPI_WIN_STACK_full(s))
	AMPI_WIN_STACK_expand(s);
    s->v[s->top] = val; 
    s->num_reqs=s->num_reqs+1;
    (s->top)=(s->top)+1;    
}

AMPI_WinRequest AMPI_WIN_STACK_pop(AMPI_Win_stack *s) {
  /* We do not shrink the stack. We rather keep the maximum allocation */
    /*if(empty(s))*/
    /*shrink(s);*/
    (s->top)=(s->top)-1;
    s->num_reqs=s->num_reqs-1;
    return (s->v[s->top]);
}

void AMPI_WIN_STACK_stack_init(AMPI_Win_stack *s) {
    s->top=0;
    s->v = malloc(sizeof(AMPI_WinRequest)*AMPI_WINDOW_STACK_CHUNK_SIZE);
    s->size=AMPI_WINDOW_STACK_CHUNK_SIZE;
    s->num_reqs=0;
}

void AMPI_WIN_STACK_destroy(AMPI_Win_stack *s) {
    s->top=0;
    free(s->v);
    s->size=AMPI_WINDOW_STACK_CHUNK_SIZE;
}

int AMPI_WIN_STACK_full(AMPI_Win_stack *s) {
    return (s->top >= s->size);
}

void AMPI_WIN_STACK_expand(AMPI_Win_stack *s) {
    AMPI_WinRequest *tmp;
    s->size=s->size+AMPI_WINDOW_STACK_CHUNK_SIZE;
    tmp=realloc(s->v,s->size*sizeof(AMPI_WinRequest));
    if(tmp != NULL) {
	s->v = tmp;
    }
}

void AMPI_WIN_STACK_shrink(AMPI_Win_stack *s) {
    AMPI_WinRequest *tmp;
    s->size=s->size-AMPI_WINDOW_STACK_CHUNK_SIZE;
    tmp=realloc(s->v,s->size*sizeof(AMPI_WinRequest));
    if(tmp != NULL) {
	s->v = tmp;
    }
}

int AMPI_WIN_STACK_empty(AMPI_Win_stack *s) {
    return (s->top <= s->size-AMPI_WINDOW_STACK_CHUNK_SIZE);
}

void AMPI_WIN_sync(AMPI_Win win) {
  /*
  int size=win.req_stack->num_reqs;
  int i=0;
  */
  /*printf("num_reqs: %d\n", size);*/
  /*for(i=0;i<size;i=i+1) {*/
  /*}*/

}
