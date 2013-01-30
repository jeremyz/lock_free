/*
 * File     : lf_cas.h
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 2009/11/01
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

#ifndef _LF_CAS_H_
#define _LF_CAS_H_

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

struct _lf_pointer_t
{
   _cas_field_t pointer;
   _cas_field_t counter;
} __attribute__ (( __aligned__( _cas_aligned ) ));

typedef struct _lf_pointer_t lf_pointer_t;

#define lf_counter_set(p,v)   ((p).counter=(_cas_field_t)(v))
#define lf_pointer_set(p,v)   ((p).pointer=(_cas_field_t)(v))
#define lf_eq(p0,p1)          ((p0).pointer==(p1).pointer && (p0).counter==(p1).counter)
#define lf_pointer_null(p)    ((p).pointer==(_cas_field_t)NULL)

static inline int lf_cas(volatile lf_pointer_t* mem, lf_pointer_t old_val, lf_pointer_t new_val)
{
   char success;

#if defined(__i386__) && defined(__GNUC__)

   asm volatile("lock cmpxchg8b (%6);"
         "setz %7;"
         : "=a" ( old_val.pointer )
         , "=d" ( old_val.counter )
         : "0"  ( old_val.pointer )
         , "1"  ( old_val.counter )
         , "b"  ( new_val.pointer )
         , "c"  ( new_val.counter )
         , "r"  ( mem )
         , "m"  ( success )
         : "cc", "memory"
         );

#elif defined(__x86_64__) && defined(__GNUC__)

   asm volatile (
         "lock cmpxchg16b %1;"
         "setz %0;"
         : "=q" ( success )
         , "+m" ( *mem )
         , "+d" ( old_val.counter )
         , "+a" ( old_val.pointer )
         : "c"  ( new_val.counter )
         , "b"  ( new_val.pointer )
         : "cc", "memory"
         );
#endif

   return (int)success;
}

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif /* _LF_CAS_H_ */

