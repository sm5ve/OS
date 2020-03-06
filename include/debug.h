#ifndef DEBUG
#define DEBUG

#include <elf/dwarf.h>

extern DWARF* ksyms;

void stackTrace(uint32_t skip = 0);
void plainStackTrace(uint32_t skip = 0);
void prettyStackTrace(uint32_t skip = 0);

#endif
