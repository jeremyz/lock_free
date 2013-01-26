/*
 * File     : cas.c
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

#include "cas.h"
#include <stdio.h>
#include <stdlib.h>

static void _check(int cond, const char *msg)
{
   if(!cond)
     {
        fprintf(stderr,"%s\n",msg);
        exit(EXIT_FAILURE);
     }
}

int main( int argc, char*argv[], char*env[] )
{
   int ret;
   int v1, v2, v3, v4, v5;

   cas_pointer_t mem, old_val, new_val;

   if((int)sizeof(cas_pointer_t)==16)
     {
        printf("running _x86_64_ code\n");
     }
   if((int)sizeof(cas_pointer_t)==8)
     {
        printf("running i686 code\n");
     }

   _hi(old_val,&v1); _lo(old_val,&v2);
   _hi(new_val,&v3); _lo(new_val,&v4);

   _hi(mem,&v1); _lo(mem,&v2);
   ret = cas(&mem, old_val, new_val);
   _check((ret==1)," 1 return value is wrong");
   _check((_hi_eq(mem,&v3)),"1 hi value is wrong");
   _check((_lo_eq(mem,&v4)),"1 lo value is wrong");

   _hi(mem,&v1); _lo(mem,&v5);
   ret = cas(&mem, old_val, new_val);
   _check((ret==0),"2 return value is wrong");
   _check((_hi_eq(mem,&v1)),"2 hi value is wrong");
   _check((_lo_eq(mem,&v5)),"2 lo value is wrong");

   _hi(mem,&v5); _lo(mem,&v1);
   ret = cas(&mem, old_val, new_val);
   _check((ret==0),"3 return value is wrong");
   _check((_hi_eq(mem,&v5)),"3 hi value is wrong");
   _check((_lo_eq(mem,&v1)),"3 lo value is wrong");

   printf("success\n");

   return EXIT_SUCCESS;
}

