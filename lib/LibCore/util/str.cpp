#include <util/str.h>

const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";

size_t strlen(const char* str)
{
	size_t out = 0;
	while (*str) {
		out++;
		str++;
	}
	return out;
}

char* strcpy(char* dest, const char* src)
{
	char* out = dest;
	while (*src) {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = 0;
	return out;
}

char* strcat(char* dest, const char* src)
{
	char* out = dest;
	while (*dest) {
		dest++;
	}
	while (*src) {
		*dest = *src;
		dest++;
		src++;
	}
	return out;
}

int strcmp(const char* s1, const char* s2)
{
	while (*s1 != 0 || *s2 != 0) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		s1++;
		s2++;
	}
	return 0;
}

bool streq(const char* s1, const char* s2, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		if (s1[i] != s2[i]) {
			return false;
		}
	}
	return true;
}
