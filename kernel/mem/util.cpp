#include <mem.h>

void memset(void* ptr, uint8_t val, size_t size)
{
	uint8_t* buff = (uint8_t*)ptr;
	for (size_t i = 0; i < size; i++) {
		buff[i] = val;
	}
}

void memcpy(void* dest, void* src, size_t size)
{
	uint8_t* sbuff = (uint8_t*)src;
	uint8_t* dbuff = (uint8_t*)dest;
	for (size_t i = 0; i < size; i++) {
		dbuff[i] = sbuff[i];
	}
}
