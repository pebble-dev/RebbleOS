/* stdarg.h
 *
 * Author: Wes Filardo <nwf@cs.jhu.edu>
 * Public domain; optionally see LICENSE
 */

#ifndef _STDARG_H_
#define _STDARG_H_

/* This is awful, but really these are compiler intrinsics, so we use the
 * GNU compiler intrinsics.
 */

#ifdef __GNUC__

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
#endif

typedef __builtin_va_list va_list;
#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)
#else
#error "Don't know how to use varargs not on GNUC, sorry."
#endif

#endif
