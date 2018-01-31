/* stdarg.h
 *
 * Author: Hermann Noll <hermann.noll@hotmail.com>
 * Public domain; optionally see LICENSE
 */

#ifndef _UTIL_H
#define _UTIL_H

#define STRUCT_OFF(parent_struct, member, expr) ((parent_struct*)( (char*)(expr) - __builtin_offsetof(parent_struct, member) ))

#endif
