#ifndef DEBUG
#define DEBUG

#include <elf/dwarf.h>

extern DWARF* ksyms;

void stackTrace();
void prettyStackTrace();

#endif
