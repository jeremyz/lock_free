/*
 * File     : lf_portable_cas.h
 * Author   : Jérémy Zurcher  <jeremy@asynk.ch>
 * Date     : 05/01/10
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

#ifndef LF_PORTABLE_CAS_H_
#define LF_PORTABLE_CAS_H_

#if __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ >= 1050
    #define CompareAndSwapInt(ptr,old_value, new_value) OSAtomicCompareAndSwapInt(old_value, new_value, ptr)
    #define CompareAndSwapPointer(ptr,new_value,old_value) OSAtomicCompareAndSwapPtr (old_value, new_value, ptr)
#elif defined(_MSC_VER)
    #define CompareAndSwapInt(ptr,old_value,new_value) InterlockedCompareExchange(ptr, new_value, old_value)
    #define CompareAndSwapPointer(ptr,new_value,old_value) InterlockedCompareExchange(ptr, new_value, old_value)
#elif (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 40100
    #define CompareAndSwapInt(ptr,old_value,new_value) __sync_bool_compare_and_swap(ptr, old_value, new_value)
    #define CompareAndSwapPointer(ptr,new_value,old_value) __sync_val_compare_and_swap(ptr, old_value, new_value)
#else
#  error No implementation
#endif

# endif /* LF_PORTABLE_CAS_H_ */
