#ifndef DWARF_HEADER
#define DWARF_HEADER
#include <elf/elf.h>
#include <klib/ds/Intervals.h>
#include <klib/ds/Vector.h>



class DWARF{
public:
	DWARF(ELF&);
	~DWARF();
private:
	
};

#endif
