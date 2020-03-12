#ifndef DEBUG
#define DEBUG

#include <elf/dwarf.h>

extern DWARF* ksyms;

void stackTrace(uint32_t skip = 0);
void plainStackTrace(uint32_t skip = 0);
void prettyStackTrace(uint32_t skip = 0);

void prettyStackTraceFromInterrupt(uint32_t ebp, uint32_t fault_eip);
#endif
