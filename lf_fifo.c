/*
 * File     : lf_fifo.c
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 2009/11/01
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

void lf_fifo_init( lf_fifo_t *fifo )
{
   lf_pointer_set(fifo->head,NULL);
   lf_pointer_set(fifo->tail,NULL);
   lf_counter_set(fifo->head,0);
   lf_counter_set(fifo->tail,0);
}

void lf_fifo_push( lf_fifo_t *fifo, lf_pointer_t *node )
{
   lf_pointer_t tail;
   lf_pointer_t last;
   lf_pointer_t new_node;

   lf_pointer_set(*node,NULL);
   lf_counter_set(*node,0);

   lf_pointer_set(new_node,node);
   lf_counter_set(new_node,1);

   for(;;)
     {
        tail = fifo->tail;
        if (lf_pointer_null(tail))
          {
             /* link new_node as first node */
             if (lf_cas(&fifo->tail, tail, new_node))
               {
                  lf_cas(&fifo->head, tail, new_node);
                  break;
               }
          }
        else
          {
             last = *((lf_pointer_t*)tail.pointer);
             if (lf_pointer_null(last))
               {
                  /* link new_node as last node */
                  if (lf_cas((lf_pointer_t*)(fifo->tail.pointer), last, new_node))
                    {
                       break;
                    }
               }
             else
               {
                  /* try to swing tail to the next node */
                  lf_counter_set(last,tail.counter+1);
                  lf_cas(&fifo->tail, tail, last);
               }
          }
     }
   /* try to swing tail to the new node */
   lf_counter_set(new_node,tail.counter+1);
   lf_cas(&fifo->tail, tail, new_node);
}

/* pop a node from the head of q */
lf_pointer_t* lf_fifo_pop( lf_fifo_t *fifo )
{
   lf_pointer_t head;
   lf_pointer_t tail;
   lf_pointer_t tmp;
   lf_pointer_t *node;

   for(;;)
     {
        if (lf_pointer_null(fifo->head))
          {
             return NULL;
          }

        head = fifo->head;
        tail = fifo->tail;
        if (lf_eq(tail,head))
          {
             /* try to swing tail to the next node */
             tmp = *((lf_pointer_t*)tail.pointer);
             lf_counter_set(tmp,tail.counter+1);
             lf_cas(&fifo->tail, tail, tmp);
          }

        node = (lf_pointer_t*)head.pointer;
        tmp = *node;
        /* try to swing head to the next node */
        lf_counter_set(tmp,head.counter+1);
        if ( lf_cas(&fifo->head, head, tmp) ) break;
     }
   return node;
}

