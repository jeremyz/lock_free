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

#define _cas_field_t uint32_t
#define _cas_aligned 8

#elif defined(__x86_64__) && defined(__GNUC__)

#define _cas_field_t uint64_t
#define _cas_aligned 16

#else
#error "cas not implemented yet for your arch"
#endif

struct _cas_pointer_t
{
    _cas_field_t lo;
    _cas_field_t hi;
} __attribute__ (( __aligned__( _cas_aligned ) ));

typedef struct _cas_pointer_t cas_pointer_t;

#define _hi(p,v)    ((p).hi=(_cas_field_t)(v))
#define _lo(p,v)    ((p).lo=(_cas_field_t)(v))
#define _hi_eq(p,v) ((p).hi==(_cas_field_t)(v))
#define _lo_eq(p,v) ((p).lo==(_cas_field_t)(v))

inline int cas(volatile cas_pointer_t* mem, cas_pointer_t old_val, cas_pointer_t new_val)
{
    char success;

#if defined(__i386__) && defined(__GNUC__)

    asm volatile("lock cmpxchg8b (%6);"
            "setz %7;"
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
            "lock cmpxchg16b %1;"
            "setz %0;"
            : "=q" ( success )
            , "+m" ( *mem )
            , "+d" ( old_val.hi )
            , "+a" ( old_val.lo )
            : "c"  ( new_val.hi )
            , "b"  ( new_val.lo )
            : "cc", "memory"
            );
#endif

    return (int)success;
}

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _CAS_H_ */

