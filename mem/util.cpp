#include <mem.h>

void memset(void* ptr, uint8_t val, size_t size){
	for(int i = 0; i < size; i++){
		*((uint8_t*)(ptr + i)) = val;
	}
}
