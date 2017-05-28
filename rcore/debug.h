#ifndef __DEBUG_H
#define __DEBUG_H

#define __S(x) #x
#define __S_(x) __S(x)
#define S__LINE__ __S_(__LINE__)

#define assert(x) do { if (!(x)) { panic("Assertion failed: " #x " (" __FILE__ ":" S__LINE__ ")"); } } while(0)

void panic(const char *s);

#endif
