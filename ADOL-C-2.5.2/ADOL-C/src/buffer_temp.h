/*----------------------------------------------------------------------------
 ADOL-C -- Automatic Differentiation by Overloading in C++
 File:     struct_buf.h
 Revision: $Id: buffer_temp.h 439 2013-12-02 13:30:22Z kulshres $
 Contents: - template class for linked list of Type buffers with constant length
             per buffer
           - intended to be used with structs
 
 Copyright (c) Andreas Kowarz, Kshitij Kulshreshtha, Jean Utke
  
 This file is part of ADOL-C. This software is provided as open source.
 Any use, reproduction, or distribution of the software constitutes 
 recipient's acceptance of the terms of the accompanying license file.
   
----------------------------------------------------------------------------*/

#if !defined(ADOLC_STRUCT_BUF_H)
#define ADOLC_STRUCT_BUF_H 1

#include <adolc/common.h>
#include "taping_p.h"

#if defined(__cplusplus)
/****************************************************************************/
/*                                                          This is all C++ */

#include <cstdlib>

#define BUFFER Buffer<SubBufferElement, _subBufferSize>
#define BUFFER_TEMPLATE template<class SubBufferElement, IndexType _subBufferSize>

typedef locint IndexType;

BUFFER_TEMPLATE class Buffer {

    typedef void (*InitFunctionPointer) (SubBufferElement *subBufferElement);

    static void zeroAll(SubBufferElement* subBufferElement);

    typedef struct SubBuffer {
        SubBufferElement elements[_subBufferSize];
        struct SubBuffer *nextSubBuffer;
    }
    SubBuffer;

public:
    inline Buffer() {
        firstSubBuffer = NULL;
        numEntries = 0;
        subBufferSize = _subBufferSize;
        initFunction = zeroAll;
    }
    inline Buffer(InitFunctionPointer _initFunction) {
        firstSubBuffer = NULL;
        numEntries = 0;
        subBufferSize = _subBufferSize;
        initFunction = _initFunction;
    }
    inline ~Buffer();

    inline void init(InitFunctionPointer _initFunction) {
        initFunction = _initFunction;
    }
    SubBufferElement *append();
    SubBufferElement *getElement(IndexType index);

private:
    SubBuffer *firstSubBuffer;
    InitFunctionPointer initFunction;
    IndexType subBufferSize;
    IndexType numEntries;
};

BUFFER_TEMPLATE
void BUFFER::zeroAll(SubBufferElement* subBufferElement) {
    memset(subBufferElement,0,sizeof(*subBufferElement));
}

BUFFER_TEMPLATE
BUFFER::~Buffer() {
    SubBuffer *tmpSubBuffer = NULL;

    while (firstSubBuffer != NULL) {
        tmpSubBuffer = firstSubBuffer;
        firstSubBuffer = firstSubBuffer->nextSubBuffer;
        delete tmpSubBuffer;
    }
}

BUFFER_TEMPLATE
SubBufferElement *BUFFER::append() {
    SubBuffer *currentSubBuffer=firstSubBuffer, *previousSubBuffer=NULL;
    IndexType index, tmp=numEntries;

    while (tmp>=subBufferSize) {
        previousSubBuffer=currentSubBuffer;
        currentSubBuffer=currentSubBuffer->nextSubBuffer;
        tmp-=subBufferSize;
    }
    if (currentSubBuffer==NULL) {
        currentSubBuffer=new SubBuffer;
        if (firstSubBuffer==NULL) firstSubBuffer=currentSubBuffer;
        else previousSubBuffer->nextSubBuffer=currentSubBuffer;
        currentSubBuffer->nextSubBuffer=NULL;
    }
    index=tmp;

    if (initFunction!=NULL)
        initFunction(&(currentSubBuffer->elements[index]));

    currentSubBuffer->elements[index].index=numEntries;
    ++numEntries;

    return &currentSubBuffer->elements[index];
}

BUFFER_TEMPLATE
SubBufferElement *BUFFER::getElement(IndexType index) {
    SubBuffer *currentSubBuffer=firstSubBuffer;

    if (index>=numEntries) fail(ADOLC_BUFFER_INDEX_TO_LARGE);
    while (index>=subBufferSize) {
        currentSubBuffer=currentSubBuffer->nextSubBuffer;
        index-=subBufferSize;
    }
    return &currentSubBuffer->elements[index];
}

#endif /* __cplusplus */

#endif /* ADOLC_STRUCT_BUF_H */

