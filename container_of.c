/*
 * File     : container_of.c
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

#define container_of(ptr, type, member) ({		 \
   const typeof( ((type *)0)->member ) *__mptr = (ptr);	 \
   (type *)( (char *)__mptr - offsetof(type,member) );})

#define containerof(ptr, type, member) ( { (type*)(((char*)ptr)-offsetof(type,member)); } )

struct node
{
   int data1;
   int data2;
   int data3;
   int data4;
   int data5;
};

static void _check(int cond, const char *msg)
{
   if(!cond)
     {
        fprintf(stderr,"%s\n",msg);
        exit(EXIT_FAILURE);
     }
}

int main(int argc, char *argv[])
{
   struct node n;

   _check((&n==container_of( &n.data1, struct node, data1)),"1 container_of failed");
   _check((&n==container_of( &n.data2, struct node, data2)),"2 container_of failed");
   _check((&n==container_of( &n.data3, struct node, data3)),"3 container_of failed");
   _check((&n==container_of( &n.data4, struct node, data4)),"4 container_of failed");
   _check((&n==container_of( &n.data5, struct node, data5)),"5 container_of failed");

   _check((&n==containerof( &n.data1, struct node, data1)),"1 container_of failed");
   _check((&n==containerof( &n.data2, struct node, data2)),"2 container_of failed");
   _check((&n==containerof( &n.data3, struct node, data3)),"3 container_of failed");
   _check((&n==containerof( &n.data4, struct node, data4)),"4 container_of failed");
   _check((&n==containerof( &n.data5, struct node, data5)),"5 container_of failed");

   printf("success\n");

   return EXIT_SUCCESS;
}
