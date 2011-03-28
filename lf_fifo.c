/*
 * File     : lf_fifo.c
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 01/11/09 
 * License  :
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "lf_fifo.h"
#include <stdlib.h>

/* initialize an empty lf_fifo structure */
void lf_fifo_init( lf_fifo_t *q ) {
    q->head.ptr = q->tail.ptr = NULL;
    q->head.count = q->tail.count = 0;
}

/* push a node at the tail of q */
void lf_fifo_push( lf_fifo_t *q, lf_pointer_t *node ) {
    lf_pointer_t tail;
    lf_pointer_t last;
    lf_pointer_t tmp;

    /* init node */
    node->ptr = NULL;
    node->count = 0;

    /* set tmp to point to node */
    tmp.ptr = node;
    tmp.count = 1;

    for(;;) {
        /* snapshot tail */
        tail = q->tail;
        /* nothing in tail link as first node */
        if ( tail.ptr==NULL && cas( &q->tail, tail, tmp ) ) {       /* TODO right place ?? */
            q->head.ptr = node;
            /* q->head.count = 1; ??? */
            return;
        } else {
            /* snapshot last through tail */
            last = *tail.ptr;
            /* if tail is still consistant */
            if lf_eql(tail,q->tail) {
                /* if last is the last node */
                if (last.ptr == NULL) {
                    tmp.count = last.count+1;
                    if ( cas( tail.ptr, last, tmp ) ) break;
                } else {
                    /* try to swing tail to the next node */
                    last.count = tail.count+1;
                    cas( &q->tail, tail, last );
                }
            }
        }
    }
    /* try to swing tail to the next node */
    tmp.count = tail.count+1;
    cas( &q->tail, tail, tmp );
}

/* pop a node from the head of q */
lf_pointer_t* pop( lf_fifo_t *q ) {
    lf_pointer_t head;
    lf_pointer_t tail;
    lf_pointer_t tmp;
    lf_pointer_t *node;

    for(;;) {
        /* snapshot head */
        head = q->head;
        /* return NULL if queue is empty */
        if (head.ptr == NULL) { return NULL; }                      /* TODO before snapshot using q-> ?? */
        /* snapshot tail */
        tail = q->tail;
        /* if there is only one node in the queue */
        if ( tail.ptr == head.ptr ) {
            /* tail points to NULL */
            tmp.ptr = NULL;
            tmp.count = tail.count+1;
            cas( &q->tail, tail, tmp );
        } else {
            /* get the node ptr */
            node = (lf_pointer_t*)head.ptr;
            /* get the next head ready */
            tmp.ptr = node->ptr;
            tmp.count = head.count+1;
            if ( cas( &q->head, head, tmp ) ) break;
        }
    }
    return node;
}

