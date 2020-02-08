#ifndef assert

#include <panic.h>

#define assert(x, msg) (x ? (void)0 : panic(msg, __FILE__, __func__,  __LINE__));
#endif
