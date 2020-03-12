#ifndef PANIC
#define PANIC

#include <arch/i386/proc.h>

void panic(const char* message);
void panic(const char* message, const char* filename, const char* func, int lineno);
void panic(const char* message, registers regs);

#endif
