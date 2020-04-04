#include <assert.h>

extern "C" void __cxa_pure_virtual()
{
	assert(false, "Called pure virtual method");
}
