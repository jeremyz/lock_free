/*
 * File     : lf_fifo_test.c
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 2013/01/30
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

#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>

#include "lf_fifo.h"

typedef struct _node_t
{
   uint32_t data;
   lf_pointer_t link;
} node_t;

typedef struct _thread_params_t
{
   uint32_t n;
   lf_fifo_t *fifo;
   node_t *nodes;
} thread_params_t;

static void _failure(const char *msg)
{
   fprintf(stderr,"%s\n",msg);
   exit(EXIT_FAILURE);
}

static void _check(int cond, const char *msg)
{
   if(!cond)
     {
        fprintf(stderr,"%s\n",msg);
        exit(EXIT_FAILURE);
     }
}

static uint64_t time_diff(struct timespec *t0, struct timespec *t1)
{
   return ((t1->tv_sec * 1000000000) + t1->tv_nsec) - ((t0->tv_sec * 1000000000) + t0->tv_nsec);
}

static void report( char* op, uint32_t threads, uint32_t nodes, uint64_t dt)
{
   uint32_t n = threads*nodes;
   fprintf(stdout," - %s: %3d threads * %6d op in %4d [ms] : %7d [us] : %10d [ns]\t -> %6d [ns/op]\n",
            op, threads, nodes, (int)(dt/1000000), (int)(dt/1000), (int)dt, (int)(dt/n));
}

static uint32_t fifo_length(lf_fifo_t *fifo)
{
   uint32_t l = 0;
   lf_pointer_t* link;

   link = (lf_pointer_t*)fifo->head.pointer;
   while(link)
     {
        l++;
        link = (lf_pointer_t*)link->pointer;
     }
   return l;
}

void* aggressive_push( void* param )
{
   uint32_t i;
   lf_fifo_t *fifo;
   node_t *nodes;
   thread_params_t *params;

   params = (thread_params_t*)param;
   fifo = params->fifo;
   nodes = params->nodes = malloc( sizeof(node_t)*params->n);
   if (nodes==NULL) _failure("nodes malloc failure");

   for(i=0; i<params->n; i++)
     {
        nodes[i].data = i+1;
        lf_fifo_push(fifo,&nodes[i].link);
     }

   return NULL;
}

void* aggressive_pop( void* param )
{
   uint32_t i;
   lf_fifo_t *fifo;
   thread_params_t *params;

   params = (thread_params_t*)param;
   fifo = params->fifo;

   i = 0;
   for(;;)
     {
        if(lf_fifo_pop(fifo)==NULL) break;
        i++;
     }
   params->n = i;

   return NULL;
}

static void run_aggressive_push_pop(uint32_t threads_n, uint32_t nodes_n)
{
   uint32_t i, j;
   lf_fifo_t fifo;
   pthread_t *threads;
   thread_params_t *params;
   struct timespec start, end;

   threads = malloc( sizeof(pthread_t)*threads_n);
   if (threads==NULL) _failure("threads malloc failure");
   params = malloc( sizeof(thread_params_t)*threads_n);
   if (params==NULL) _failure("params malloc failure");

   lf_fifo_init( &fifo);

   for(i=0; i<threads_n; i++)
     {
        params[i].fifo = &fifo;
        params[i].n = nodes_n;
     }

   clock_gettime(CLOCK_MONOTONIC, &start);
   for(i=0; i<threads_n; i++)
     {
        if (pthread_create( &threads[i], NULL, aggressive_push, &params[i]))
          _failure("Failed to create thread");
     }
   for(i=0; i<threads_n; i++)
     {
        pthread_join( threads[i], NULL );
     }
   clock_gettime(CLOCK_MONOTONIC, &end);

   report( "aggressive push", threads_n, nodes_n, time_diff( &start, &end ));
   _check((fifo_length(&fifo)==(threads_n*nodes_n)),"fifo length failure after aggressive push");

   clock_gettime(CLOCK_MONOTONIC, &start);
   for(i=0; i<threads_n; i++)
     {
        if (pthread_create( &threads[i], NULL, aggressive_pop, &params[i]))
          _failure("Failed to create thread");
     }
   for(i=0; i<threads_n; i++)
     {
        pthread_join( threads[i], NULL );
     }
   clock_gettime(CLOCK_MONOTONIC, &end);

   report( "aggressive pop", threads_n, nodes_n, time_diff( &start, &end ));
   _check((fifo_length(&fifo)==0),"fifo length failure after aggressive pop");

   j = 0;
   for(i=0; i<threads_n; i++)
     {
        j += params[i].n;
        free(params[i].nodes);
     }
   _check((j==(threads_n*nodes_n)),"poped nodes count failure");

   free(threads);
   free(params);
}

int main(int argc, char *argv[])
{
   run_aggressive_push_pop(100,100000);
   /* run_aggressive_push_pop(1,10); */

   printf("success\n");

   return EXIT_SUCCESS;
}
