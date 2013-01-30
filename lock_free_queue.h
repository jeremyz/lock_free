/*
 * File     : lock_free_queue.h
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 2009/11/01
 * License  : stolen from http://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf
 */

#ifndef  _LOCK_FREE_QUEUE_H_
#define _LOCK_FREE_QUEUE_H_

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

struct node;

typedef union pointer
{
   struct
     {
        volatile struct node *ptr;
        volatile unsigned int count;
     } split;
   volatile unsigned long long concat;
} pointer_t;

typedef struct node
{
   void *data;
   pointer_t next;
} node_t;

typedef struct queue
{
   pointer_t head;
   pointer_t tail;
} lfq_t;

void init(lfq_t *q);
void enqueue(lfq_t *q, void *data);
void* dequeue(lfq_t *q);

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _LOCK_FREE_QUEUE_H_ */
