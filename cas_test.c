/*
 * File     : atomic.c
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

#include <stdio.h>
#include "cas.h"

#define MAKE_LONG_LONG(lo, hi)		((hi)<<32)+(lo)

typedef union pointer {
    struct {
        volatile void *ptr;
        volatile unsigned int count;
    } split;
    volatile unsigned long long concat;
} pointer_t;

int main( int argc, char*argv[], char*env[] ) {
    int ret;

    pointer_t mem, old, new;
    
    mem.split.count = 0;
    old.split.count = 6;
    new.split.count = 666;
    mem.split.ptr = (void*)&argc;
    old.split.ptr = (void*)&argc;
    new.split.ptr = (void*)&argv;

    ret = compare_and_swap(&mem.concat, old.concat, new.concat);
    printf("ret %d -> (%d,%X)\n", ret, mem.split.count, (unsigned int)mem.split.ptr);

    mem.split.count=6;
    ret = compare_and_swap(&mem.concat, old.concat, new.concat);
    printf("ret %d -> (%d,%X)\n", ret, mem.split.count, (unsigned int)mem.split.ptr);

    return 0;
}

