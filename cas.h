/*
 * File     : cas.h
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 25/01/13
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

#ifndef _CAS_H_
#define _CAS_H_

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

#include <stdint.h>

#if defined(__i386__) && defined(__GNUC__)

struct _cas_pointer_t
{
    uint32_t lo;
    uint32_t hi;
} __attribute__ (( __aligned__( 8 ) ));

#define _hi(p,v)    ((p).hi=(uint32_t)(v))
#define _lo(p,v)    ((p).lo=(uint32_t)(v))
#define _hi_eq(p,v) ((p).hi==(uint32_t)(v))
#define _lo_eq(p,v) ((p).lo==(uint32_t)(v))

#elif defined(__x86_64__) && defined(__GNUC__)

struct _cas_pointer_t
{
    uint64_t lo;
    uint64_t hi;
} __attribute__ (( __aligned__( 16 ) ));

#define _hi(p,v)    ((p).hi=(uint64_t)(v))
#define _lo(p,v)    ((p).lo=(uint64_t)(v))
#define _hi_eq(p,v) ((p).hi==(uint64_t)(v))
#define _lo_eq(p,v) ((p).lo==(uint64_t)(v))

#else
#error "cas not implemented yet for your arch"
#endif

typedef struct _cas_pointer_t cas_pointer_t;

inline int cas(volatile cas_pointer_t* mem, cas_pointer_t old_val, cas_pointer_t new_val)
{
    char success;

#if defined(__i386__) && defined(__GNUC__)

    asm volatile("lock; cmpxchg8b (%6);"
            "setz %7; "
            : "=a" ( old_val.lo ),
              "=d" ( old_val.hi )
            : "0"  ( old_val.lo ),
              "1"  ( old_val.hi ),
              "b"  ( new_val.lo ),
              "c"  ( new_val.hi ),
              "r"  ( mem ),
              "m"  ( success )
            : "cc", "memory"
            );

#elif defined(__x86_64__) && defined(__GNUC__)

    asm volatile (
            "lock cmpxchg16b %1\n\t"
            "setz %0"
            : "=q" ( success )
            , "+m" ( *mem )
            , "+d" ( old_val.hi )
            , "+a" ( old_val.lo )
            : "c"  ( new_val.hi )
            , "b"  ( new_val.lo )
            : "cc"
            );
#endif

    return (int)success;
}

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _CAS_H_ */

