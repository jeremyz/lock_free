/*
 * File     : lf_ringbuffer.h
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 05/01/010
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

#ifndef _LF_RING_BUFFER_H_
#define _LF_RING_BUFFER_H_

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

#include <sys/types.h>
#include "lf_ringbuffer_data.h"

#define LFRB_NO_BLOCK   1       /* if buffer is full, leave instead of try again and again */
#define IS_NOT_BLOCKING( flags ) ( (flags)&LFRB_NO_BLOCK )

/* the ring buffer structure */
typedef struct ringbuffer {
    LFRB_BUFFER_TYPE *buffer;   /* buffer data */
    size_t n_buf;               /* number of buffers, max 65534, see implementation for details */
    unsigned int indexes;       /* indexes where to read_from and write_to */
} lf_ringbuffer_t;

/* return an initialized lf_ringbuffer_t struct, size is limited to 65534, see implementation for details */
lf_ringbuffer_t* lf_ringbuffer_create( size_t n_buf );

/* destroy an lf_ringbuffer_t struct */
void lf_ringbuffer_destroy( lf_ringbuffer_t *r );

/* return 1 if is empty */
int lf_ringbuffer_empty( lf_ringbuffer_t *r );

/* return the count of filled buffers */
size_t lf_ringbuffer_read_size( lf_ringbuffer_t *r );

/* return the count of available buffers */
size_t lf_ringbuffer_write_size( lf_ringbuffer_t *r );

/* write data into the ring buffer
 * return 0 on success
 * return -1 if IS_NOT_BLOCKING and buffer is full
 */
int lf_ringbuffer_write( lf_ringbuffer_t *r, void *data, int flags );

/* read data from the ring buffer
 * return 0 on success
 * return -1 if IS_NOT_BLOCKING and buffer is empty
 */
int lf_ringbuffer_read( lf_ringbuffer_t *r, void *data, int flags );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _LF_RING_BUFFER_H_ */
