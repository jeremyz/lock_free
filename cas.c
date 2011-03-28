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

#include <stdio.h>

typedef struct split {
    volatile void *ptr;
    volatile unsigned int count;
} split_t;

typedef union pointer {
    split_t split;
    volatile unsigned long long concat;
} pointer_t;


/* CMPXCHG8B m64   Compare EDX:EAX with m64. If equal, set ZF and load ECX:EBX into m64. Else, clear ZF and load m64 into EDX:EAX. */
static inline unsigned int compare_and_swap(volatile unsigned long long *mem,
                                            volatile unsigned long long old,
                                            volatile unsigned long long new) {
    char result;
    __asm__ __volatile__("lock; cmpxchg8b %0; setz %1;"
            : "=m"(*mem), "=q"(result)
            : "m"(*mem), "d" ((unsigned long)(old>>32)), "a" ((unsigned long)old),
            "c" ((unsigned long)(new>>32)), "b" ((unsigned long)new)
            : "memory");
    return (int)result;
}

/* CMPXCHG8B m64   Compare EDX:EAX with m64. If equal, set ZF and load ECX:EBX into m64. Else, clear ZF and load m64 into EDX:EAX. */
static inline unsigned int cas( volatile split_t *mem,
                                volatile split_t old,
                                volatile split_t new ) {
    char result;
    __asm__ __volatile__("lock; cmpxchg8b %0; setz %1;"
            : "=m"(*mem), "=q"(result)
            : "m"(*mem), "d" (old.count), "a" (old.ptr),
            "c" (new.count), "b" (new.ptr)
            : "memory");
    return (int)result;
}


int test_compare_and_swap () {
    pointer_t mem, old, new;
    new.concat = old.concat = 0;
    return compare_and_swap(&mem.concat, old.concat, new.concat);
}

int test_cas () {
    pointer_t mem, old, new;
    new.concat = old.concat = 0;
    return cas(&mem.split, old.split, new.split);
}

void check_assign_a() {
    pointer_t a, b;
    b.concat = 0;
    a.concat = b.concat;
}

void check_assign_b() {
    pointer_t a, b;
    a=b;
}

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

    mem.split.count = 0;
    old.split.count = 6;
    new.split.count = 666;
    mem.split.ptr = (void*)&argc;
    old.split.ptr = (void*)&argc;
    new.split.ptr = (void*)&argv;

    ret = cas(&mem.split, old.split, new.split);
    printf("ret %d -> (%d,%X)\n", ret, mem.split.count, (unsigned int)mem.split.ptr);

    mem.split.count=6;
    ret = cas(&mem.split, old.split, new.split);
    printf("ret %d -> (%d,%X)\n", ret, mem.split.count, (unsigned int)mem.split.ptr);

    return 0;
}

