#ifndef DWARF_HEADER
#define DWARF_HEADER
#include <elf/elf.h>
#include <klib/ds/Intervals.h>
#include <klib/ds/Vector.h>
#include <mem.h>

struct __attribute__((packed)) DWARF_aranges_header{
	uint32_t length;
	uint16_t version;
	uint32_t info_offset;
	uint8_t addr_size;
	uint8_t seg_size;
};

class DWARFRange{
public:
	DWARFRange(IntervalSet<uint32_t>*, uint32_t offset, uint16_t version);
	DWARFRange();
	~DWARFRange();
	IntervalSet<uint32_t>* ranges;
	uint32_t info_offset;
	uint16_t version;	
};

class DWARF{
public:
	DWARF(ELF&);
	~DWARF();
private:
	Vector<DWARFRange> ranges;
};

#endif
