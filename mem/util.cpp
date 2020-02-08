#include <mem.h>

void memset(void* ptr, uint8_t val, size_t size){
	for(int i = 0; i < size; i++){
		*((uint8_t*)(ptr + i)) = val;
	}
}

void memcpy(void* dest, void* src, size_t size){
	for(int i = 0; i < size; i++){
		*((uint8_t*)(dest + i)) = *((uint8_t*)(src + i));
	}
}
