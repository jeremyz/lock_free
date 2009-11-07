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

#define container_of(ptr, type, member) ( { (type*)(((char*)ptr)-offsetof(type,member)); } )

struct node {
    int data;
    lf_pointer_t link;
};

int main(int argc, char *argv[]) {
    lf_pointer_t new;
    lf_pointer_t old;
    lf_pointer_t mem;
    int ret;

    lf_fifo_t q;
    lf_pointer_t *it;
    struct node data[10];
    int i;

    /* check comprae_and_swap */
    mem.count = 0;
    old.count = 6;
    new.count = 666;
    mem.ptr = (void*)&argc;
    old.ptr = (void*)&argc;
    new.ptr = (void*)&argv;
    ret = cas(&mem, old, new);
    printf("ret %d -> (%d,%X)\n", ret, mem.count, (unsigned int)mem.ptr);

    mem.count=6;
    ret = cas(&mem, old, new);
    printf("ret %d -> (%d,%X)\n", ret, mem.count, (unsigned int)mem.ptr);

    /* init data */
    for(i=0; i<10; i++) data[i].data=i;
    for(i=0; i<10; i++) printf("data[%d] :%d\n",i,data[i].data);
   
    /* check lf_fifo */
    lf_fifo_init( &q);
    printf("pop %X\n",(unsigned int)pop( &q ));
    for(i=0; i<10; i++) lf_fifo_push( &q, &data[i].link );

    it = (lf_pointer_t*)q.head.ptr;
    while(it!=NULL) {
        printf("data : %d\n",container_of(it,struct node,link)->data);
        it = (lf_pointer_t*)it->ptr;
    }
    
    for(i=0; i<5; i++) {
        it = pop( &q );
        printf("pop %X %d\n",(unsigned int)it,container_of(it,struct node,link)->data);
    }
    it = (lf_pointer_t*)q.head.ptr;
    while(it!=NULL) {
        printf("data : %d\n",container_of(it,struct node,link)->data);
        it = (lf_pointer_t*)it->ptr;
    }

    return EXIT_SUCCESS;
}
