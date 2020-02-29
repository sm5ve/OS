#include <elf/elf.h>
#include <assert.h>
#include <klib/SerialDevice.h>
#include <stddef.h>

ELF::ELF(void* b){
	base = b;
			
	ELFHeader32* header = getHeader32();
	assert((header -> magic[0] == 0x7f) && (header -> magic[1] == 'E') && (header -> magic[2] == 'L') && (header -> magic[3] == 'F'), "Error: invalid ELF header");
	assert(header -> elf_class == ELF_CLASS_32, "Error: we do not support 64 bit binaries yet!");
	assert(header -> osabi == ELF_OSABI_SYSTEM_V, "Error: erroneous OS ABI");
	assert(header -> architecture == ELF_ARCH_386, "Error: we're on a 386 ya dingus!");
	assert(getSectionHeader(header -> section_header_string_tbl_index) == getSectionHeader(".shstrtab"), "Error: strtab mismatch");
}

ELF::~ELF(){

}

ELFHeader32* ELF::getHeader32(){
	return (ELFHeader32*)base;
}

ELFSectionHeader32* ELF::getSectionHeader(uint32_t index){
	assert(index < getHeader32() -> section_header_entry_count, "Error: tried to index out of bounds section");
	uint32_t out = (uint32_t) base + getHeader32() -> section_header_off + index * getHeader32() -> section_header_entry_size;
	return (ELFSectionHeader32*) out;
}

String ELF::getSectionNameForIndex(uint32_t index){
	ELFSectionHeader32* shstrtab = getSectionHeader(getHeader32() -> section_header_string_tbl_index);
	char* ptr = (char*)((uint32_t)base + shstrtab -> offset);
	char* end = ptr + shstrtab -> size;
	assert(&ptr[index] < end, "Error: attempted to get out of bounds ELF string");
	return String(&ptr[index]);
}

String ELF::getStringForIndex(uint32_t index){
	ELFSectionHeader32* strtab = getSectionHeader(".strtab");
	char* ptr = (char*)((uint32_t)base + strtab -> offset);
	char* end = ptr + strtab -> size;
	assert(&ptr[index] < end, "Error: attempted to get out of bounds ELF string");
	return String(&ptr[index]);
}

ELFSectionHeader32* ELF::getSectionHeader(String sectionName){
	for(int i = 0; i < getHeader32() -> section_header_entry_count; i++){
		ELFSectionHeader32* section = getSectionHeader(i);
		if(getSectionNameForIndex(section -> name_index) == sectionName){
			return section;
		}
	}
	return NULL;
}

PrintStream& operator<<(PrintStream& p, const ELFHeader32* h){
	p << "ELF class:\t\t\t";
	switch(h -> elf_class){
		case ELF_CLASS_NONE: p << "None\n"; break;
		case ELF_CLASS_32: p << "ELF32\n"; break;
		case ELF_CLASS_64: p << "ELF64\n"; break;
		default: p << (int)(h -> elf_class) << " (unknown)\n"; break;
	}
	p << "Data encoding:\t\t\t";
	switch(h -> data){
		case ELF_DATA_NONE: p << "None\n"; break;
		case ELF_DATA2_LSB: p << "2's compliment little endian\n"; break;
		case ELF_DATA2_MSB: p << "2's compliment big endian\n"; break;
		default: p << (int)(h -> data) << " (unknown)\n"; break;
	}
	p << "OS/ABI:\t\t\t\t";
	switch(h -> osabi){
		case ELF_OSABI_SYSTEM_V: p << "System V\n"; break;
		case ELF_OSABI_HPUX: p << "HP UX\n"; break;
		case ELF_OSABI_NETBSD: p << "NetBSD\n"; break;
		case ELF_OSABI_LINUX: p << "Linux\n"; break;
		case ELF_OSABI_SOLARIS: p << "Solaris\n"; break;
		case ELF_OSABI_AIX: p << "AIX\n"; break;
		case ELF_OSABI_IRIX: p << "IRIX\n"; break;
		case ELF_OSABI_FREEBSD: p << "FreeBSD\n"; break;
		case ELF_OSABI_TRU64: p << "Tru64\n"; break;
		case ELF_OSABI_MODESTO: p << "Modesto\n"; break;
		case ELF_OSABI_OPENBSD: p << "OpenBSD\n"; break;
		case ELF_OSABI_OPENVMS: p << "OpenVMS\n"; break;
		case ELF_OSABI_NSK: p << "NSK\n"; break;
		default: p << h -> osabi << " (unknown)\n"; break;
	}
	p << "ABI Version:\t\t\t" << h -> abiversion << "\n";
	p << "Version:\t\t\t" << h -> version;
	switch(h -> version){
		case 1: p << " (current)\n"; break;
		default: p << " (unknown)\n"; break;
	}
	p << "Type:\t\t\t\t";
	switch(h -> type){
		case ELF_TYPE_NONE: p << "None\n"; break;
		case ELF_TYPE_RELOCATABLE: p << "Relocatable\n"; break;
		case ELF_TYPE_EXEC: p << "Executable\n"; break;
		case ELF_TYPE_DYNAMIC: p << "Dynamic\n"; break;
		case ELF_TYPE_CORE: p << "Core\n"; break;
		default: p << h -> type << " (unknown)\n"; break;
	}
	p << "Architecture:\t\t\t";
	switch(h -> architecture){
		case ELF_ARCH_386: p << "i386\n"; break;
		case ELF_ARCH_X86_64: p << "x86_64\n"; break;
		case ELF_ARCH_ARM: p << "ARM\n"; break;
		default: p << h -> type << "\n"; break;
	}
	p << "Entry point (virtual address):\t" << (void*)(h -> entry_vaddr) << "\n";
	p << "Start of section headers:\t" << (h -> section_header_off) << " bytes from start\n";
	p << "Section header entry size:\t" << (h -> section_header_entry_size) << " bytes\n";
	p << "Section header entry count:\t" << (h -> section_header_entry_count) << "\n";

	return p;
}
