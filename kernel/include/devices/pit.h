#ifndef PIT_header
#define PIT_header

#include <stdint.h>

namespace PIT{
	void initOneshot();
	void waitMicros(uint32_t);
	void waitMillis(uint32_t);
}

#endif
