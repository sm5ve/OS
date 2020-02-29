#ifndef DWARF_HEADER
#define DWARF_HEADER

#include <elf/elf.h>

class DWARF{
public:
	DWARF(ELF&);
	~DWARF();
};

#endif
