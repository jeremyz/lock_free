/*
 * File     : lf_fifo_test.c
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 01/11/09 
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
#include "stddef.h"

#include "lf_fifo.h"
#include "lf_fifo_cas.h"

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

struct node {
    int data;
    pointer_t link;
};

int main(int argc, char *argv[]) {
    pointer_t new;
    pointer_t old;
    pointer_t mem;
    int ret;

    lf_fifo_t q;
    pointer_t *it;
    struct node data[10];
    int i;

    /* check comprae_and_swap */
    mem.split.count = 0;
    old.split.count = 6;
    new.split.count = 666;
    mem.split.next = (void*)&argc;
    old.split.next = (void*)&argc;
    new.split.next = (void*)&argv;
    ret = cas(&mem.split, old.split, new.split);
    printf("ret %d -> (%d,%X)\n", ret, mem.split.count, (unsigned int)mem.split.next);

    mem.split.count=6;
    ret = cas(&mem.split, old.split, new.split);
    printf("ret %d -> (%d,%X)\n", ret, mem.split.count, (unsigned int)mem.split.next);

    /* init data */
    for(i=0; i<10; i++) data[i].data=i;
    for(i=0; i<10; i++) printf("data[%d] :%d\n",i,data[i].data);
   
    /* check lf_fifo */
    lf_fifo_init( &q);
    printf("pop %X\n",(unsigned int)pop( &q ));
    for(i=0; i<10; i++) lf_fifo_push( &q, &data[i].link );

    it = (pointer_t*)q.head.split.next;
    while(it!=NULL) {
        printf("data : %d\n",container_of(it,struct node,link)->data);
        it = (pointer_t*)it->split.next;
    }
    
    for(i=0; i<5; i++) {
        it = pop( &q );
        printf("pop %X %d\n",(unsigned int)it,container_of(it,struct node,link)->data);
    }
    it = (pointer_t*)q.head.split.next;
    while(it!=NULL) {
        printf("data : %d\n",container_of(it,struct node,link)->data);
        it = (pointer_t*)it->split.next;
    }

    return EXIT_SUCCESS;
}
