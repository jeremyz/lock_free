/*
 * File     : lf_ring_buffer.c
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

#include "lf_ring_buffer.h"
#include "lf_portable_cas.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* initialize an empty lf_ring_buffer struct */
lf_ring_buffer_t* lf_ring_buffer_create( int n_buf ) {
    /* alloc ring_buffer struct */
    lf_ring_buffer_t *r = malloc(sizeof(lf_ring_buffer_t));
    if(r==NULL) return NULL;
    /* */
    r->buffer = malloc(sizeof(lf_buffer_el_t)*n_buf);
    if(r->buffer==NULL) {
        free(r);
        return NULL;
    }
    memset(r->buffer,0,sizeof(lf_buffer_el_t)*r->n_buf);
    r->n_buf = n_buf;
    r->read_from = -1;
    r->write_to = 0;
    r->write_delay=BACKOFF_DELAY_INIT;
    r->read_delay=BACKOFF_DELAY_INIT;
    return r;
}

/* destroy an lf_ring_buffer strcture */
void lf_ring_buffer_destroy( lf_ring_buffer_t *r ) {
    free(r->buffer);
    free(r);
}

/* write data into the ring buffer */
int lf_ring_buffer_write( lf_ring_buffer_t *r, rb_data_t *data, int flags ) {
    int idx;
    int next;
    struct timespec st;
    st.tv_sec=0;
    /* reserve a buffer */
    for(;;){
        idx = r->write_to;
        if(IS_AVAILABLE(idx) ) {
            /* read_from==idx means that the buffer is full and that a writer thread which at first reserved this buffer
             * hasn't had enough CPU cycles to call MARK_AS_FILLED
             */
            if(!r->read_from==idx) {
                next = (idx+1)%r->n_buf;
                /* what might have happend between IS_AVAILABLE and now :
                * - a writter has reserved this buffer => write_to has moved => CAS fails
                * - a reader has consumed a buffer => read_from has moved => we've got more space
                */
                if( CompareAndSwapInt( &r->write_to, idx, next ) ) break;
            }
            /* TODO simply reloop, nothing to do ??? */
        } else {
            if(IS_NOT_BLOCKING(flags)) return -1;
            st.tv_nsec=r->write_delay;
            nanosleep(&st,NULL);
            if(r->write_delay<BACKOFF_DELAY_MAX) r->write_delay+=BACKOFF_DELAY_INC;
        }
    }
    /* try to set read_from on idx if it has not been initialized yet
     * !!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!
     * there is a really tiny chance if the ring is too small and there is a lot of writers that,
     * another thread might have reserved this same buffer before this
     * which means this thread and the other will write data to this current buffer !!!!
     */
    if(r->read_from==-1) CompareAndSwapInt( &r->read_from, -1, idx );
    /* fill this buffer and mark it as filled */
    memcpy( r->buffer[idx].data, data, RB_DATA_SIZE*sizeof(rb_data_t) );
    MARK_AS_FILLED( idx );
    if(r->write_delay>BACKOFF_DELAY_INIT) r->write_delay-=BACKOFF_DELAY_INC;
    return 0;
}

/* read data from the ring buffer */
int lf_ring_buffer_read( lf_ring_buffer_t *r, rb_data_t *data, int flags ) {
    int idx;
    struct timespec st;
    st.tv_sec=0;
    if(r->read_from==-1) return -1;
    for(;;) {
        idx = r->read_from;
        if(!IS_AVAILABLE(idx)) {
            memcpy( data, r->buffer[idx].data, RB_DATA_SIZE*sizeof(rb_data_t) );
            if( CompareAndSwapInt( &r->read_from, idx, (idx+1)%r->n_buf ) ) break;
        } else { 
            if(IS_NOT_BLOCKING(flags)) return -1;
            st.tv_nsec=r->read_delay;
            nanosleep(&st,NULL);
            if(r->read_delay<BACKOFF_DELAY_MAX) r->read_delay+=BACKOFF_DELAY_INC;
        }
    }
    /* finish the read process */
    MARK_AS_READ( idx );
    if(r->read_delay>BACKOFF_DELAY_INIT) r->read_delay-=BACKOFF_DELAY_INC;
    return 0;
}

