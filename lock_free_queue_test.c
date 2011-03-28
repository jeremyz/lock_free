/*
 * File     : lock_free_queue_test.c
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
#include "lock_free_queue.h"

int main(int argc, char *argv[]) {
    lfq_t q;
    node_t *it;
    int data[10];
    int i;

    for(i=0; i<10; i++) data[i]=i;
    for(i=0; i<10; i++) printf("data[i] :%d (%X)\n",data[i],(unsigned int)&data[i]);
    
    init( &q);
    for(i=0; i<10; i++) enqueue( &q, (void*)&data[i] );

    it = (node_t*)q.head.split.ptr;
    while(it!=NULL) {
        if(it->data>0)printf("data : %X %d\n",(unsigned int)it->data,*((int*)it->data));
        it = (node_t*)it->next.split.ptr;
    }
    
    for(i=0; i<5; i++) printf("unqueue %X\n",(unsigned int)dequeue( &q ));
    it = (node_t*)q.head.split.ptr;
    while(it!=NULL) {
        if(it->data>0)printf("data : %X %d\n",(unsigned int)it->data,*((int*)it->data));
        it = (node_t*)it->next.split.ptr;
    }


    return EXIT_SUCCESS;
}
