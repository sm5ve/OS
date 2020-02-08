#ifndef PANIC
#define PANIC

void [[noreturn]] panic(const char* message);
void [[noreturn]] panic(const char* message, const char* filename, const char* func, int lineno);

#endif
