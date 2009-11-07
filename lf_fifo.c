/*
 * File     : lfq.c
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
#include "lf_fifo_cas.h"
#include "stdlib.h"

/* initialize an empty lf_fifo structure */
void lf_fifo_init( lf_fifo_t *q ) {
    q->head.split.next = q->tail.split.next = NULL;
    q->head.split.count = q->tail.split.count = 0;
}

/* push a node at the tail of q */
void lf_fifo_push( lf_fifo_t *q, pointer_t *node ) {
    pointer_t tail;
    pointer_t last;
    pointer_t tmp;

    /* init node */
    node->split.next = NULL;
    node->split.count = 0;

    /* set tmp to point to node */
    tmp.split.next = node;
    tmp.split.count = 1;

    for(;;) {
        /* snapshot tail */
        tail.concat = q->tail.concat;
        /* nothing in tail link as first node */
        if ( tail.split.next==NULL && cas( &q->tail.split, tail.split, tmp.split ) ) {                  /* TODO right place ?? */
            q->head.split.next = node;
            /* q->head.split.count = 1; ??? */
            return;
        } else {
            /* snapshot last through tail */
            last.concat = (*tail.split.next).concat;
            /* if tail is still consistant */
            if (tail.concat == q->tail.concat) {
                /* if last is the last node */
                if (last.split.next == NULL) {
                    tmp.split.count = last.split.count+1;
                    if ( cas( &tail.split.next->split, last.split, tmp.split ) ) break;
                } else {
                    /* try to swing tail to the next node */
                    last.split.count = tail.split.count+1;
                    cas( &q->tail.split, tail.split, last.split );
                }
            }
        }
    }
    /* try to swing tail to the next node */
    tmp.split.count = tail.split.count+1;
    cas( &q->tail.split, tail.split, tmp.split );
}

/* pop a node from the head of q */
pointer_t* pop( lf_fifo_t *q ) {
    pointer_t head;
    pointer_t tail;
    pointer_t tmp;
    pointer_t *node;

    for(;;) {
        /* snapshot head */
        head.concat = q->head.concat;
        /* return NULL if queue is empty */
        if (head.split.next == NULL) { return NULL; }                                                   /* TODO before snapshot using q-> ?? */
        /* snapshot tail */
        tail.concat = q->tail.concat;
        /* if there is only one node in the queue */
        if ( tail.split.next == head.split.next ) {
            /* tail points to NULL */
            tmp.split.next = NULL;
            tmp.split.count = tail.split.count+1;
            cas( &q->tail.split, tail.split, tmp.split );
        } else {
            /* get the node ptr */
            node = (pointer_t *)head.split.next;
            /* get the next head ready */
            tmp.split.next = node->split.next;
            tmp.split.count = head.split.count+1;
            if ( cas( &q->head.split, head.split, tmp.split ) ) break;
        }
    }
    return node;
}

