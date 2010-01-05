/*
 * File     : lf_ring_buffer.h
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

#define BACKOFF_DELAY_INIT  1000
#define BACKOFF_DELAY_INC   3000
#define BACKOFF_DELAY_MAX   90000

#define NO_BLOCK    1       /* if buffer is full, leave instead of try again and again */
#define IS_NOT_BLOCKING( flags ) ( (flags)&NO_BLOCK )

#define RB_DATA_SIZE    63
typedef char rb_data_t;

typedef struct buffer_el {
    char status;
    rb_data_t data[RB_DATA_SIZE];
} lf_buffer_el_t;

typedef struct ring_buffer {
    lf_buffer_el_t *buffer; /* buffer data */
    int n_buf;              /* number of buffers */
    int read_from;          /* index where to read data from */
    int write_to;           /* index where to write data to */
    int write_delay;        /* backoff nanosleep to reduce fast looping when writing */
    int read_delay;         /* backoff nanosleep to reduce fast looping when reading */
} lf_ring_buffer_t;

#define IS_AVAILABLE( idx ) (r->buffer[(idx)].status==0)
#define MARK_AS_FILLED( idx ) { r->buffer[(idx)].status=1; }
#define MARK_AS_READ( idx ) { r->buffer[(idx)].status=0; }

/* initialize an empty lf_ring_buffer struct */
lf_ring_buffer_t* lf_ring_buffer_create( int n_buf );

/* destroy an lf_ring_buffer strcture */
void lf_ring_buffer_destroy( lf_ring_buffer_t *r );

/* write data into the ring buffer
 * return 0 on success
 * return -1 if IS_NOT_BLOCKING and buffer is full
 */
int lf_ring_buffer_write( lf_ring_buffer_t *r, rb_data_t *data, int flags );

/* read data from the ring buffer
 * return 0 on success
 * return -1 if IS_NOT_BLOCKING and buffer is empty
 */
int lf_ring_buffer_read( lf_ring_buffer_t *r, rb_data_t *data, int flags );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _LF_RING_BUFFER_H_ */