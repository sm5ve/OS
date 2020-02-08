#include <klib/util/str.h>

const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";

void itoa(int value, char* str, int base){
	if(value == 0){
		str[0] = '0';
		str[1] = 0;
	}
	if(value < 0){
		str[0] = '-';
		str++;
		value *= -1;
	}
	int len = 0;
	while(value != 0){
		str[len] = digits[value % base];
		value /= base;
		len++;
	}
	for(int i = 0; i < len/2; i++){
		char a = str[i];
		char b = str[len - i - 1];
		str[len - i - 1] = a;
		str[i] = b;
	}
	str[len] = 0;
}
