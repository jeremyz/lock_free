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

#if !defined LFRB_DATA_SIZE
    #error "LFRB_DATA_SIZE is not defined"
#endif
#if !defined LFRB_BUFFER_TYPE
    #error "LFRB_BUFFER_TAPE is not defined"
#endif
#if !defined LFRB_BUFFER_SIZE
    #error "LFRB_BUFFER_SIZE is not defined"
#endif
#if !defined LFRB_IS_AVAILABLE
    #error "LFRB_IS_AVAILABLE is not defined"
#endif
#if !defined LFRB_MARK_AS_FILLED
    #error "LFRB_MARK_AS_FILLED is not defined"
#endif
#if !defined LFRB_MARK_AS_READ
    #error "LFRB_MARK_AS_READ is not defined"
#endif
#if !defined LFRB_DATA_PTR
    #error "LFRB_DATA_PTR is not defined"
#endif

//#define DEBUG_LFR_RING 1

#ifdef DEBUG_LFR_RING
    #include <stdio.h>
    #define _LOG_( ... ) fprintf(stdout,__VA_ARGS__)
#else
    #define _LOG_( ... )
#endif

#define BACKOFF_NANO_SLEEP  100

/* initialize an empty lf_ring_buffer struct */
lf_ring_buffer_t* lf_ring_buffer_create( size_t n_buf ) {
    /* alloc ring_buffer struct */
    lf_ring_buffer_t *r = malloc(sizeof(lf_ring_buffer_t));
    if(r==NULL) return NULL;
    /* */
    r->buffer = malloc(LFRB_BUFFER_SIZE*n_buf);
    if(r->buffer==NULL) {
        free(r);
        return NULL;
    }
    memset(r->buffer,0,LFRB_BUFFER_SIZE*n_buf);
    r->n_buf = n_buf;
    r->read_from = -1;
    r->write_to = 0;
    return r;
}

/* destroy an lf_ring_buffer strcture */
void lf_ring_buffer_destroy( lf_ring_buffer_t *r ) {
    free(r->buffer);
    free(r);
}

/* write data into the ring buffer */
int lf_ring_buffer_write( lf_ring_buffer_t *r, void *data, int flags ) {
    int idx, next;
    struct timespec backoff;
    /* reserve a buffer */
    for(;;){
        idx = r->write_to;
        if(LFRB_IS_AVAILABLE( r->buffer[idx] ) ) {
            /* read_from==idx means that the buffer is full and that a writer thread which at first reserved this buffer
             * hasn't had enough CPU cycles to call MARK_AS_FILLED
             */
            if(!(r->read_from==idx)) {
                next = idx+1;
                if (next==r->n_buf) next=0;
                /* what might have happend between IS_AVAILABLE and now :
                * - a writter has reserved this buffer => write_to has moved => CAS fails
                * - a reader has consumed a buffer => read_from has moved => we've got more space
                */
                _LOG_( "write: CAS %d %d %d\n", r->write_to, idx, next );
                if( CompareAndSwapInt( &r->write_to, idx, next ) ) break;
            } else {
                /* TODO simply reloop, nothing to do ??? */
                // TODO ERROR when all has been read
                _LOG_("write: buffer full %d %d\n",r->read_from,idx);
            }
        } else {
            _LOG_("write: not available\n");
            if(IS_NOT_BLOCKING(flags)) return -1;
            backoff.tv_sec = 0;
            backoff.tv_nsec = BACKOFF_NANO_SLEEP;
            nanosleep(&backoff,NULL);
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
    memcpy( r->buffer[idx].data, data, LFRB_DATA_SIZE );
    LFRB_MARK_AS_FILLED( r->buffer[idx] );
    return 0;
}

/* read data from the ring buffer */
int lf_ring_buffer_read( lf_ring_buffer_t *r, void *data, int flags ) {
    int idx, next;
    struct timespec backoff;
    if(r->read_from==-1) return -1;
    for(;;) {
        idx = r->read_from;
        if( !(LFRB_IS_AVAILABLE( r->buffer[idx] )) ) {
            next = idx+1;
            if (next==r->n_buf) next=0;
            /* will do bad things if data dst buffer is too small !! */
            memcpy( data, r->buffer[idx].data, LFRB_DATA_SIZE );
            _LOG_( "read: CAS %d %d %d\n", r->read_from, idx, next );
            if( CompareAndSwapInt( &r->read_from, idx, next ) ) {
                if(r->read_from==r->write_to) {
                    /* the buffer is actually empty but writers will see it as full */
                    _LOG_( "read: empty CAS %d %d %d\n", r->read_from, next, -1 );
                    CompareAndSwapInt( &r->read_from, next, -1 );
                }
                break;
            }
        } else { 
            _LOG_("read: not available\n");
            if(IS_NOT_BLOCKING(flags)) return -1;
            backoff.tv_sec = 0;
            backoff.tv_nsec = BACKOFF_NANO_SLEEP;
            nanosleep(&backoff,NULL);
        }
    }
    /* finish the read process */
    LFRB_MARK_AS_READ( r->buffer[idx] );
    return 0;
}

