#ifndef DWARF_HEADER
#define DWARF_HEADER
#include <elf/elf.h>
#include <klib/ds/Intervals.h>
#include <klib/ds/Vector.h>
#include <klib/ds/Tuple.h>
#include <klib/ds/HashMap.h>
//#include <mem.h>

#define CU_NOT_FOUND 0xffffffff

struct __attribute__((packed)) DWARF_aranges_header{
	uint32_t length;
	uint16_t version;
	uint32_t info_offset;
	uint8_t addr_size;
	uint8_t seg_size;
};

struct __attribute__((packed)) DWARF_info_header{
	uint32_t length;
	uint16_t version;
	uint32_t abbrev_offset;
	uint8_t addr_size;
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

class DWARFSchema{
public:
	DWARFSchema(void*& ptr);
	uint32_t abbrev_code;
	uint32_t tag_type;
	bool has_children;
	Vector<Tuple<uint32_t, uint32_t>> fields;
};

class DWARFDIE{
public:
	DWARFDIE(void*& ptr, HashMap<uint32_t, DWARFSchema*>*, ELF*);
	void* value(uint32_t name);
private:
	HashMap<uint32_t, void*> map;
};

class DWARFLineStateMachine{
public:
	DWARFLineStateMachine(void*& ptr);
	Maybe<Tuple<uint32_t, char*>> getLineForAddr(void*);
private:
	void* statements_start;
	void* section_end;
	bool default_it_stmt;
	uint8_t min_inst_len;
	int8_t line_base;
	uint8_t line_range;
	uint8_t opcode_base;
	
	uint32_t addr;
	uint32_t op_index;
	uint32_t file;
	uint32_t line;
	uint32_t col;
	bool is_stmt;
	bool basic_block;
	bool end_seq;
	bool prologue_end;
	bool epilogue_begin;
	uint32_t isa;
	uint32_t discriminator;

	void reset();
	void advance();
};

class DWARF{
public:
	DWARF(ELF*);
	~DWARF();

	Tuple<uint32_t, char*> getLineForAddr(void*);
private:
	Vector<DWARFRange> ranges;
	ELF* elf;
	uint32_t getCUOffsetForAddr(void* ptr);
	HashMap<uint32_t, DWARFSchema*>* decodeAbbrev(uint32_t offset);
};

#endif
