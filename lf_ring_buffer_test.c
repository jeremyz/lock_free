#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lf_ring_buffer.h"

//#define BUFFER_LEN 5000000
#define BUFFER_LEN 500

static rb_data_t data[BUFFER_LEN][RB_DATA_LEN];

static int64_t time_diff(struct timespec *t0, struct timespec *t1)
{
    return ((t1->tv_sec * 1000000000) + t1->tv_nsec) - ((t0->tv_sec * 1000000000) + t0->tv_nsec);
}

static void print_now(char* s) {
    fprintf(stdout,s);
    fflush(stdout);
}

static void report( int n, uint64_t dt ) {
    fprintf(stdout,"%d operations in %d [ms] => %d [us] => %d [ns]\t >>> %d [ns/op]\n", n, (int)(dt/1000000), (int)(dt/1000), (int)dt, (int)(dt/n) );
}

static void feed_data( int n){
    int i;
    for(i=0; i<n; i++){
        sprintf(data[i],"hello world %04d\n",i);
    }
}

static uint64_t sequential_writes( lf_ring_buffer_t *ring, int n, int flags ) {
    int i;
    rb_data_t data[RB_DATA_LEN];
    struct timespec start, end;
    
    print_now("sequential writes ... ");

    clock_gettime(CLOCK_MONOTONIC, &start);

    for(i=0; i<n; i++) lf_ring_buffer_write( ring, data, flags );

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("done.\n");
    
    return time_diff( &start, &end );
}

static uint64_t sequential_reads( lf_ring_buffer_t *ring, int n, int flags ) {
    int i;
    rb_data_t data[RB_DATA_LEN];
    struct timespec start, end;
    
    print_now("sequential reads ... ");

    clock_gettime(CLOCK_MONOTONIC, &start);

    for(i=0; i<n; i++) lf_ring_buffer_read( ring, data, flags );

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("done.\n");
    
    return time_diff( &start, &end );
}

int main( int argc, char** argv ) {
    
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

    report( b_len, sequential_writes( ring, b_len, 0 ) );
    report( b_len, sequential_reads( ring, b_len, 0 ) );
    report( b_len, sequential_writes( ring, b_len, 0 ) );
    report( b_len, sequential_reads( ring, b_len, 0 ) );

    lf_ring_buffer_destroy( ring );

    return EXIT_SUCCESS;
}

/*
#define ARRAY_SIZE 2
#define MAX_VALUE 0x10000

jack_ringbuffer_t *rb;
volatile int flowing = 0;

static int
fill_int_array (int *array, int start, int count)
{
  int i, j = start;
  for (i = 0; i < count; i++)
  {
    array[i] = j;
    j = (j + 1) % MAX_VALUE;
  }
  return j;
}

static int
cmp_array (int *array1, int *array2, int count)
{
  int i;
  for (i = 0; i < count; i++)
    if (array1[i] != array2[i])
    {
      printf("%d != %d at offset %d\n", array1[i], array2[i], i);
      fflush(stdout);
      return 0;
    }

  return 1;      
}

static void *
reader_start (void * arg)
{
  int i = 0, a[ARRAY_SIZE], b[ARRAY_SIZE];
  unsigned long j = 0, nfailures = 0;
  printf("[reader started] ");
  fflush(stdout);
  i = fill_int_array (a, i, ARRAY_SIZE);
  while (1)
  {
    if (j == 100 && !flowing)
    {
      printf("[flowing] ");
      fflush(stdout);
      flowing = 1;
    }

    if (jack_ringbuffer_read_space (rb) >= (int) ARRAY_SIZE * (int) sizeof (int))
    {
      if (jack_ringbuffer_read (rb, (char *) b, ARRAY_SIZE * sizeof (int)))
      {
        if (!cmp_array (a, b, ARRAY_SIZE))
        {
          nfailures++;
          //
          //printf("failure in chunk %lu - probability: %lu/%lu = %.3f per million\n", 
          //       j, nfailures, j, (float) nfailures / (j + 1) * 1000000);
          //i = (b[0] + ARRAY_SIZE) % MAX_VALUE;
          //
          printf("FAILURE in chunk %lu\n", j);
          fflush(stdout);
          exit(1);
        }
        i = fill_int_array (a, i, ARRAY_SIZE);
        j++;
      }
    }
  }

  return NULL;    
}

static void *
writer_start (void * arg)
{
  int i = 0, a[ARRAY_SIZE];
  printf("[writer started] ");
  fflush(stdout);

  i = fill_int_array (a, i, ARRAY_SIZE);

  while (1)
  {
    if (jack_ringbuffer_write_space (rb) >= (int) ARRAY_SIZE * (int) sizeof (int))
    {
      if (jack_ringbuffer_write (rb, (char *) a, ARRAY_SIZE * sizeof (int)))
      {
        i = fill_int_array (a, i, ARRAY_SIZE);
      }
    }
  }        

  return NULL;    
}

int main(int argc, char *argv[])
{
  int size;
  sscanf(argv[1], "%d", &size);
  printf("starting (120s max) - array/buffer size: %d/%d\n", 
         (int) sizeof(int) * ARRAY_SIZE, size);
  fflush(stdout);
  rb = jack_ringbuffer_create(size);
  pthread_t reader_thread, writer_thread;
  if (pthread_create (&reader_thread, NULL, reader_start, NULL))
  {
    printf("Failed to create reader thread\n");
    exit(1);
  }
  if (pthread_create (&writer_thread, NULL, writer_start, NULL))
  {
    printf("Failed to create writer thread\n");
    exit(1);
  }
  sleep(120);
  if (flowing)
    printf("SUCCESS\n");
  else
    printf("FAILURE: data did not flow\n");
  fflush(stdout);
  return 0;
}
*/
