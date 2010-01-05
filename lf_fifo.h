/*
 * File     : lf_fifo.h
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 02/11/09 
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

#ifndef _LF_FIFO_H_
#define _LF_FIFO_H_

#include "lf_cas.h"

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

typedef struct queue {
    lf_pointer_t head;
    lf_pointer_t tail;
} lf_fifo_t;

/* initialize an empty lf_fifo structure */
void lf_fifo_init( lf_fifo_t *q );

/* push a node at the tail of q */
void lf_fifo_push( lf_fifo_t *q, lf_pointer_t *node );

/* pop a node from the head of q */
lf_pointer_t* pop( lf_fifo_t *q );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _LF_FIFO_H_ */

