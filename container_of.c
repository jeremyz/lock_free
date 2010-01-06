/*
 * File     : lfq_test.c
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

#define container_of(ptr, type, member) ({			        \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

struct node {
    int data1;
    int data2;
    int data3;
    int data4;
    int data5;
};

#define my_offset(ptr, type, member) ( { (type*)(((char*)ptr)-offsetof(type,member)); } )

struct node n;

struct node* use_container_of() {
    return container_of( &n.data3, struct node, data3);
}

struct node* use_mine() {
    return my_offset( &n.data3, struct node, data3);
/*    return (struct node*)(((char*)&n.daat3)-OFFSET); */
}

struct node* use_struct(){
    return &n;
}

int main(int argc, char *argv[]) {

    printf ("%X\n", (int)use_container_of());
    printf ("%X\n", (int)use_mine());
    printf ("%X\n", (int)use_struct());

    return EXIT_SUCCESS;
}
