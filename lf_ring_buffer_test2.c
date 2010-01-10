#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lf_ring_buffer.h"

#define BUFFER_LEN 65000

//static rb_data_t data[BUFFER_LEN][RB_DATA_LEN];

static int64_t time_diff(struct timespec *t0, struct timespec *t1)
{
    return ((t1->tv_sec * 1000000000) + t1->tv_nsec) - ((t0->tv_sec * 1000000000) + t0->tv_nsec);
}

/*
static void print_now(char* s) {
    fprintf(stdout,s);
    fflush(stdout);
}
*/

static void report( char* op, int n, uint64_t dt, int redo ) {
    fprintf(stdout,"\t%9d %s operations in %4d [ms] => %7d [us] => %10d [ns]\t >>> %6d [ns/op]\t%4d redone operations\n",
            n, op, (int)(dt/1000000), (int)(dt/1000), (int)dt, (int)(dt/n), redo );
}

struct thread_params {
    lf_ring_buffer_t *ring;
    int n;
    int flags;
};

/*
static void feed_data( int n){
    int i;
    for(i=0; i<n; i++){
        sprintf(data[i],"hello world %04d\n",i);
    }
}
*/

static uint64_t sequential_writes( lf_ring_buffer_t *ring, int n, int flags ) {
    int i, redo=0;
    rb_data_t data[RB_DATA_LEN];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if(flags==0) {
        for(i=0; i<n; i++) lf_ring_buffer_write( ring, data, flags );
    } else {
        for(i=0; i<n;) {
            if(lf_ring_buffer_write( ring, data, flags )==0) { i++; } else { redo+=1; }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    report( "write", n, time_diff( &start, &end ), redo );
    return time_diff( &start, &end );
}

static uint64_t sequential_reads( lf_ring_buffer_t *ring, int n, int flags ) {
    int i,redo=0;
    rb_data_t data[RB_DATA_LEN];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if(flags==0) {
        for(i=0; i<n; i++) lf_ring_buffer_read( ring, data, flags );
    } else {
        for(i=0; i<n;) {
            if(lf_ring_buffer_read( ring, data, flags )==0) { i++; } else { redo+=1; }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    report( "read ", n, time_diff( &start, &end ), redo );
    return time_diff( &start, &end );
}

void* writer_thread( void* param ) {
    struct thread_params *params = (struct thread_params*)param;
    //report( "write", params->n, sequential_writes( params->ring, params->n, params->flags ) );
    sequential_writes( params->ring, params->n, params->flags );
    return NULL;
}

void* reader_thread( void* param ) {
    struct thread_params *params = (struct thread_params*)param;
    //report( "read ", params->n, sequential_reads( params->ring, params->n, params->flags ) );
    sequential_reads( params->ring, params->n, params->flags );
    return NULL;
}

static void parallel_op( int op, int nt, lf_ring_buffer_t *ring, int n, int flags ) {
    int i;
    
    pthread_t *threads = malloc( sizeof(pthread_t)*nt);
    struct thread_params *params = malloc( sizeof(struct thread_params)*nt);

    for(i=0; i<nt; i++) {
        params[i].ring = ring;
        params[i].n = n/nt;
        params[i].flags = flags;
        if(op==0) {
            if (pthread_create( &threads[i], NULL, writer_thread, &params[i])) {
                fprintf(stderr,"Failed to create writer thread[%d]\n",i);
                exit(1);
            }
        } else if(op==1) {
            if (pthread_create( &threads[i], NULL, reader_thread, &params[i])) {
                fprintf(stderr,"Failed to create reader thread[%d]\n",i);
                exit(1);
            }
        } else {
            params[i].n /=2;
            if (pthread_create( &threads[i], NULL, writer_thread, &params[i])) {
                fprintf(stderr,"Failed to create writer thread[%d]\n",i);
                exit(1);
            }
            if (pthread_create( &threads[i], NULL, reader_thread, &params[i])) {
                fprintf(stderr,"Failed to create reader thread[%d]\n",i);
                exit(1);
            }
        }
    }
    for(i=0; i<nt; i++) {
        pthread_join( threads[i], NULL );
    }
    /* empty the ring */
    free(threads);
    free(params);
}

int main( int argc, char** argv ) {
    
    int i;
    int b_len = BUFFER_LEN;
    lf_ring_buffer_t *ring;

    ring = lf_ring_buffer_create( b_len );
    if(ring==NULL){
        fprintf(stderr,"ERROR : lf_ring_buffer_create( %d );\n",b_len);
        exit( EXIT_FAILURE );
    }

    /*
    print_now("feed the data ... ");
    feed_data(b_len);
    printf("done.\n");
    */

    printf("sequential non blocking write operations ...\n");
    sequential_writes( ring, b_len, 0 );
    printf("sequential non blocking read operations ...\n");
    sequential_reads( ring, b_len, 0 );
    if(!lf_ring_buffer_empty(ring)) { fprintf(stderr,"ring should be empty but is not\n"); exit( EXIT_FAILURE ); }
    printf("sequential blocking write operations ...\n");
    sequential_writes( ring, b_len, LFRB_NO_BLOCK );
    printf("sequential blocking read operations ...\n");
    sequential_reads( ring, b_len, LFRB_NO_BLOCK );
    if(!lf_ring_buffer_empty(ring)) { fprintf(stderr,"ring should be empty but is not\n"); exit( EXIT_FAILURE ); }

    for(i=5; i<=50;i*=2) {
        printf("%d parallel blocking with backoff inc write operations .... \n",i);
        parallel_op( 0, i, ring, b_len, 0 );
        printf("parallel blocking read operations ...\n");
        sequential_reads( ring, b_len, 0 );
        if(!lf_ring_buffer_empty(ring)) { fprintf(stderr,"ring should be empty but is not\n"); exit( EXIT_FAILURE ); }
    }
    for(i=5; i<=50;i*=2) {
        printf("%d parallel non blocking write operations .... \n",i);
        parallel_op( 0, i, ring, b_len, LFRB_NO_BLOCK );
        printf("non blocking read operations ...\n");
        sequential_reads( ring, b_len, LFRB_NO_BLOCK );
        if(!lf_ring_buffer_empty(ring)) { fprintf(stderr,"ring should be empty but is not\n"); exit( EXIT_FAILURE ); }
    }
    for(i=10; i<=50;i*=2) {
        printf("%d parallel blocking write and read operations .... \n",i*2);
        parallel_op( 3, i, ring, b_len, 0 );
        if(!lf_ring_buffer_empty(ring)) { fprintf(stderr,"ring should be empty but is not\n"); exit( EXIT_FAILURE ); }
    }

    lf_ring_buffer_destroy( ring );

    return EXIT_SUCCESS;
}

