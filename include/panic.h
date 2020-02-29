#ifndef PANIC
#define PANIC

void panic(const char* message);
void panic(const char* message, const char* filename, const char* func, int lineno);

#endif
