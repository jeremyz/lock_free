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

//#define DEBUG_LFRB_KO 1
//#define DEBUG_LFRB_CAS 1
//#define DEBUG_LFRB 1

#ifdef DEBUG_LFRB
    #include <stdio.h>
    #define _LOG_KO_( ... ) fprintf(stdout,__VA_ARGS__)
    #define _LOG_CAS_( ... ) fprintf(stdout,__VA_ARGS__)
#elif defined DEBUG_LFRB_CAS
    #include <stdio.h>
    #define _LOG_KO_( ... )
    #define _LOG_CAS_( ... ) fprintf(stdout,__VA_ARGS__)
#elif defined DEBUG_LFRB_KO
    #include <stdio.h>
    #define _LOG_KO_( ... ) fprintf(stdout,__VA_ARGS__)
    #define _LOG_CAS_( ... )
#else
    #define _LOG_KO_( ... )
    #define _LOG_CAS_( ... )
#endif

#define BACKOFF_NANO_SLEEP  100000

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

/* return 1 if is empty */
int lf_ring_buffer_empty( lf_ring_buffer_t *r ) { return r->read_from==-1; }

/* write data into the ring buffer */
int lf_ring_buffer_write( lf_ring_buffer_t *r, void *data, int flags ) {
    int write_to, read_from, next;
    struct timespec backoff;
    int backoff_time = BACKOFF_NANO_SLEEP;
    /* reserve a buffer */
    for(;;){
        write_to = r->write_to;
        read_from = r->read_from;
        if(LFRB_IS_AVAILABLE( r->buffer[write_to] ) ) {
            /* read_from==write_to means that the buffer is full and that a writer thread which at first reserved this buffer
             * hasn't had enough CPU cycles to call MARK_AS_FILLED
             */
            if( read_from!=write_to ) {
                next = write_to+1;
                if (next==r->n_buf) next=0;
                /* what might have happend between IS_AVAILABLE and now :
                * - a writter has reserved this buffer => write_to has moved => CAS fails
                * - a reader has consumed a buffer => read_from has moved => we've got more space
                */
                _LOG_CAS_( "write: CAS %d %d %d\n", r->write_to, write_to, next );
                if( CompareAndSwapInt( &r->write_to, write_to, next ) ) {
                    /* !!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!
                     * - if the ring is empty before this write operation (r->read_from==-1 or latest idx)
                     * - and n_buf other threads execute this same function before the next CAS is finished
                     * then, the last thread will :
                     * - see this buffer as available
                     * - and see the ring as empty instead of full !!!!!
                     * so it will ends with two threads writing on this buffer
                     */
                    if(r->read_from==-1) CompareAndSwapInt( &r->read_from, -1, write_to );
                    break;
                }
            } else {
                _LOG_KO_("write: ring full %d %d\n",r->read_from,idx);
                if(IS_NOT_BLOCKING(flags)) return -1;
                backoff.tv_sec = 0;
                backoff.tv_nsec = backoff_time;
                nanosleep(&backoff,NULL);
            }
        } else {
            _LOG_KO_("write: buffer not available\n");
            if(IS_NOT_BLOCKING(flags)) return -1;
            backoff.tv_sec = 0;
            backoff.tv_nsec = backoff_time;
            nanosleep(&backoff,NULL);
        }
        backoff_time += BACKOFF_NANO_SLEEP;
    }
    /* fill this buffer and mark it as filled */
    memcpy( LFRB_DATA_PTR(r->buffer[write_to]), data, LFRB_DATA_SIZE );
    LFRB_MARK_AS_FILLED( r->buffer[write_to] );
    return 0;
}

/* read data from the ring buffer */
int lf_ring_buffer_read( lf_ring_buffer_t *r, void *data, int flags ) {
    int write_to, read_from, next;
    struct timespec backoff;
    int backoff_time = BACKOFF_NANO_SLEEP;
    for(;;) {
        write_to = r->write_to;
        read_from = r->read_from;
        if( !(LFRB_IS_AVAILABLE( r->buffer[read_from] )) && read_from!=-1 ) {
            next = read_from+1;
            if (next==r->n_buf) next=0;
            /* will do bad things if data dst buffer is too small !! */
            memcpy( data, LFRB_DATA_PTR(r->buffer[read_from]), LFRB_DATA_SIZE );
            _LOG_CAS_( "read: CAS %d %d %d\n", r->read_from, read_from, next );
            if( CompareAndSwapInt( &r->read_from, read_from, next ) ) {
                if(r->read_from==r->write_to) {
                    /* the buffer is empty but writers will see it as full */
                    _LOG_CAS_( "read: empty CAS %d %d %d\n", r->read_from, next, -1 );
                    CompareAndSwapInt( &r->read_from, next, -1 );
                }
                break;
            }
        }

        _LOG_KO_("read: ring empty\n");
        if(IS_NOT_BLOCKING(flags)) return -1;
        backoff.tv_sec = 0;
        backoff.tv_nsec = backoff_time;
        nanosleep(&backoff,NULL);
        backoff_time += BACKOFF_NANO_SLEEP;
    }
    /* finish the read process */
    LFRB_MARK_AS_READ( r->buffer[read_from] );
    return 0;
}

