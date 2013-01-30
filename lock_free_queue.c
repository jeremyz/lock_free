/*
 * File     : lock_free_queue.c
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 2009/11/01
 * License  : stolen from http://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf
 */

#include "stdlib.h"
#include "lock_free_queue.h"
#ifdef DEBUG
#include "stdio.h"
#endif

/* CMPXCHG8B m64   Compare EDX:EAX with m64. If equal, set ZF and load ECX:EBX into m64. Else, clear ZF and load m64 into EDX:EAX. */
static inline unsigned int compare_and_swap(volatile unsigned long long *mem,
                                            volatile unsigned long long old,
                                            volatile unsigned long long new)
{
   char result;
   __asm__ __volatile__("lock; cmpxchg8b %0; setz %1;"
            : "=m"(*mem), "=q"(result)
            : "m"(*mem), "d" ((unsigned long)(old>>32)), "a" ((unsigned long)old)
            , "c" ((unsigned long)(new>>32)), "b" ((unsigned long)new)
            : "memory");
   return (int)result;
}

void init(lfq_t *q)
{
   node_t *node = (node_t*)malloc(sizeof(node_t));
   node->next.split.ptr = NULL;
   node->next.split.count = 0;
   q->head.split.ptr = q->tail.split.ptr = node;
   q->head.split.count = q->tail.split.count = 0;
}

void enqueue(lfq_t *q, void *data)
{
   node_t *node;
   pointer_t tail;
   pointer_t next;
   pointer_t tmp;

   node = (node_t*)malloc(sizeof(node_t));
   node->data = data;
   node->next.split.ptr = NULL;
   /* node->next.split.count = 0; */
   for(;;)
     {
        tail.concat = q->tail.concat;                 /* copy tail pointer */
        next.concat = tail.split.ptr->next.concat;    /* copy next pointer */
        if (tail.concat == q->tail.concat)
          {                                           /* tail is still consistent */
             if (next.split.ptr == NULL)
               {                                      /* next is still the last node */
                  /* if tail->next is the same as next, link node at the end of the list */
                  tmp.split.ptr = node;
                  tmp.split.count = next.split.count+1;
                  if ( compare_and_swap( &tail.split.ptr->next.concat, next.concat, tmp.concat ) ) break;
               }
             else
               {
                  /* try to swing tail to the next node, if q-> tail is still tail => next is ok */
                  tmp.split.ptr = next.split.ptr;
                  tmp.split.count = tail.split.count+1;
                  compare_and_swap( &q->tail.concat, tail.concat, tmp.concat );
               }
          }
     }
   /* try to swing tail to the next node, may have been done by a concurrent push */
   tmp.split.ptr = node;
   tmp.split.count = tail.split.count+1;
   compare_and_swap( &q->tail.concat, tail.concat, tmp.concat );
}

void* dequeue(lfq_t *q)
{
   void *data;
   pointer_t head;
   pointer_t tail;
   node_t *next;
   pointer_t tmp;

   for(;;)
     {
        head.concat = q->head.concat;
        tail.concat = q->tail.concat;
        next = (node_t *)head.split.ptr;
        if (head.concat == q->head.concat)
          {
             if (head.split.ptr == tail.split.ptr)
               {     /* still consistent */
                  if (next->next.split.ptr == NULL)
                    {     /* queue empty */
                       return NULL;
                    }
                  /* new node has been linked, but tail is behind, should advance it */
                  tmp.split.ptr = next->next.split.ptr;
                  tmp.split.count = tail.split.count+1;
                  compare_and_swap( &q->tail.concat, tail.concat, tmp.concat );
               }
             else
               {
                  data = next->data;
                  /* try to swing head to the next node */
                  tmp.split.ptr = next->next.split.ptr;
                  tmp.split.count = head.split.count+1;
                  if( compare_and_swap( &q->head.concat, head.concat, tmp.concat ) ) break;
               }
          }
     }
   free((void*)head.split.ptr);
   return data;
}

