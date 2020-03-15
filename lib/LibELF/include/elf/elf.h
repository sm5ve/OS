#ifndef ELF_HEADER
#define ELF_HEADER

#include <elf/elf_consts.h>
#include <stdint.h>
#include <PrintStream.h>
#include <ds/String.h>

struct __attribute__((packed)) ELFHeader32{
	char magic[4];
	uint8_t elf_class;
	uint8_t data;
	uint8_t version_other;
	uint8_t osabi;
	uint8_t abiversion;
	char padding[7];
	uint16_t type;	
	uint16_t architecture;
	uint32_t version;
	uint32_t entry_vaddr;
	uint32_t prog_header_off;
	uint32_t section_header_off;
	uint32_t flags;
	uint16_t elf_header_size;
	uint16_t prog_header_entry_size;
	uint16_t prog_header_entry_count;
	uint16_t section_header_entry_size;
	uint16_t section_header_entry_count;
	uint16_t section_header_string_tbl_index;
};

struct __attribute__((packed)) ELFSectionHeader32{
	uint32_t name_index;
	uint32_t type;
	uint32_t flags;
	uint32_t addr;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t addralign;
	uint32_t entry_size;
};

class ELF{
public:
	ELF(void*);
	~ELF();

	ELFHeader32* getHeader32();
	ELFSectionHeader32* getSectionHeader(uint32_t index);
	ELFSectionHeader32* getSectionHeader(String name);
	String getSectionNameForIndex(uint32_t);
	String getStringForIndex(uint32_t);

	void* getSectionBase(ELFSectionHeader32*);
private:
	void* base; 
};

PrintStream& operator<<(PrintStream&, const ELFHeader32*);

#endif
