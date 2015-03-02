/*
##########################################################
# This file is part of the AdjoinableMPI library         #
# released under the MIT License.                        #
# The full COPYRIGHT notice can be found in the top      #
# level directory of the AdjoinableMPI distribution.     #
########################################################## 
*/
#include <malloc.h>
#include <assert.h>
#include <mpi.h>
#include "ampi/bookkeeping/support.h"

struct RequestListItem { 
  struct AMPI_Request_S ampiRequest; /*[llh] I'd rather put *ampiRequest to not copy */
  struct RequestListItem *next_p;
  struct RequestListItem *prev_p;
};

static struct RequestListItem* requestListHead=0;
static struct RequestListItem* requestListTail=0;
static struct RequestListItem* unusedRequestStack=0;

/**
 * very simple implementation for now 
 */
static struct RequestListItem* addRequestToList() { 
  struct RequestListItem* returnItem_p=0;
  if (! unusedRequestStack) { 
    unusedRequestStack=(struct RequestListItem*)malloc(sizeof(struct RequestListItem));
    assert(unusedRequestStack);
    unusedRequestStack->next_p=0; 
    unusedRequestStack->prev_p=0; 
  }
  /* get it from the unused stack */
  returnItem_p=unusedRequestStack;
  unusedRequestStack=returnItem_p->prev_p;
  returnItem_p->prev_p=0;
  /* add it to the list */
  if (!requestListHead) requestListHead=returnItem_p;
  if (requestListTail) { 
    requestListTail->next_p=returnItem_p;
    returnItem_p->prev_p=requestListTail;
  }
  requestListTail=returnItem_p;
  return returnItem_p;
}

/**
 * very simple implementation for now 
 */
static void dropRequestFromList(struct RequestListItem* toBoDropped) { 
  /* remove it from the list */
  if (requestListHead==toBoDropped) { 
    requestListHead=toBoDropped->next_p;
    if (requestListHead) requestListHead->prev_p=0;
    toBoDropped->next_p=0;
  }
  if (requestListTail==toBoDropped) { 
    requestListTail=toBoDropped->prev_p;
    if (requestListTail) requestListTail->next_p=0;
    toBoDropped->prev_p=0;
  }
  if (toBoDropped->next_p && toBoDropped->prev_p) {
    toBoDropped->prev_p->next_p=toBoDropped->next_p;
    toBoDropped->next_p->prev_p=toBoDropped->prev_p;
    toBoDropped->next_p=0; 
    toBoDropped->prev_p=0;
  }
  /* add it to the unused stack */
  if (unusedRequestStack) { 
    toBoDropped->prev_p=unusedRequestStack;
  }
  unusedRequestStack=toBoDropped;
}

static struct RequestListItem* findRequestInList(MPI_Request *request, int traced) { 
  struct RequestListItem* current_p=requestListHead;
  while(current_p) { 
    if ((traced==0 && current_p->ampiRequest.plainRequest==*request) || (traced!=0 && current_p->ampiRequest.tracedRequest==*request)) break;
    current_p=current_p->next_p;
  }
  assert(current_p);
  return current_p;
}

void BK_AMPI_put_AMPI_Request(struct AMPI_Request_S  *ampiRequest) { 
  struct RequestListItem *inList_p;
  inList_p=addRequestToList();
  inList_p->ampiRequest=*ampiRequest;
}

void BK_AMPI_get_AMPI_Request(MPI_Request *request, struct AMPI_Request_S  *ampiRequest, int traced) { 
  struct RequestListItem *inList_p=findRequestInList(request,traced);
  *ampiRequest=inList_p->ampiRequest;
  dropRequestFromList(inList_p);
}

void BK_AMPI_read_AMPI_Request(MPI_Request *request, struct AMPI_Request_S  *ampiRequest, int traced) { 
  struct RequestListItem *inList_p=findRequestInList(request,traced);
  *ampiRequest=inList_p->ampiRequest;
}

struct WinListItem { 
  AMPI_Win ampiWin; /*[llh] I'd rather put *ampiWin to not copy */
  struct WinListItem *next_p;
  struct WinListItem *prev_p;
};

static struct WinListItem* winListHead=0;
static struct WinListItem* winListTail=0;
static struct WinListItem* unusedWinStack=0;

/**
 * very simple implementation for now 
 */
static struct WinListItem* addWinToList() { 
  struct WinListItem* returnItem_p=0;
  if (! unusedWinStack) { 
    unusedWinStack=(struct WinListItem*)malloc(sizeof(struct WinListItem));
    assert(unusedWinStack);
    unusedWinStack->next_p=0; 
    unusedWinStack->prev_p=0; 
  }
  /* get it from the unused stack */
  returnItem_p=unusedWinStack;
  unusedWinStack=returnItem_p->prev_p;
  returnItem_p->prev_p=0;
  /* add it to the list */
  if (!winListHead) winListHead=returnItem_p;
  if (winListTail) { 
    winListTail->next_p=returnItem_p;
    returnItem_p->prev_p=winListTail;
  }
  winListTail=returnItem_p;
  return returnItem_p;
}

/**
 * very simple implementation for now 
 */
static void dropWinFromList(struct WinListItem* toBoDropped) { 
  /* remove it from the list */
  if (winListHead==toBoDropped) { 
    winListHead=toBoDropped->next_p;
    if (winListHead) winListHead->prev_p=0;
    toBoDropped->next_p=0;
  }
  if (winListTail==toBoDropped) { 
    winListTail=toBoDropped->prev_p;
    if (winListTail) winListTail->next_p=0;
    toBoDropped->prev_p=0;
  }
  if (toBoDropped->next_p && toBoDropped->prev_p) {
    toBoDropped->prev_p->next_p=toBoDropped->next_p;
    toBoDropped->next_p->prev_p=toBoDropped->prev_p;
    toBoDropped->next_p=0; 
    toBoDropped->prev_p=0;
  }
  /* add it to the unused stack */
  if (unusedWinStack) { 
    toBoDropped->prev_p=unusedWinStack;
  }
  unusedWinStack=toBoDropped;
}

static struct WinListItem* findWinInList(MPI_Win *win) { 
  struct WinListItem* current_p=winListHead;
  while(current_p) { 
    if (*current_p->ampiWin.plainWindow==win) break;
    current_p=current_p->next_p;
  }
  assert(current_p);
  return current_p;
}

void BK_AMPI_put_AMPI_Win(AMPI_Win  *ampiWin) { 
  struct WinListItem *inList_p;
  inList_p=addWinToList();
  inList_p->ampiWin=*ampiWin;
}

void BK_AMPI_get_AMPI_Win(MPI_Win *win, AMPI_Win  *ampiWin) { 
  struct WinListItem *inList_p=findWinInList(win);
  *ampiWin=inList_p->ampiWin;
  dropWinFromList(inList_p);
}

void BK_AMPI_read_AMPI_Win(MPI_Win *win, AMPI_Win  *ampiWin) { 
  struct WinListItem *inList_p=findWinInList(win);
  *ampiWin=inList_p->ampiWin;
}

