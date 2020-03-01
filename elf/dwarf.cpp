#include <elf/dwarf.h>
#include <klib/SerialDevice.h>

DWARF::DWARF(ELF& elf){
	SD::the() << "ranges buffer at " << ranges.buffer << "\n";
//	assert(false, "");
	auto aranges = elf.getSectionHeader(".debug_aranges");
	assert(aranges != NULL, "Error: missing aranges");
	void* ptr = elf.getSectionBase(aranges);
	uint32_t end = (uint32_t)ptr + aranges -> size;
	while((uint32_t)ptr < end){
		DWARF_aranges_header header = *(DWARF_aranges_header*)ptr;
		assert(header.addr_size == 4, "Error: don't know how to handle differently sized addresses");
		assert(header.seg_size == 0, "Error: don't know how to handle segments");
		uint32_t* addrs = (uint32_t*)((uint32_t)ptr + sizeof(DWARF_aranges_header) + 4);
		ptr = (void*)((uint32_t)ptr + header.length + 4);
		auto set = new IntervalSet<uint32_t>();
		for(uint32_t i = 0; (addrs[i] != 0) || (addrs[i + 1] != 0); i += 2){
			set -> add(Interval<uint32_t>(addrs[i], addrs[i] + addrs[i + 1]));
		}
		ranges.push(DWARFRange(set, header.info_offset, header.version));
	}
	for(uint32_t i = 0; i < ranges.size(); i++){
		SD::the() << (void*)ranges[i].info_offset << "\n";
		SD::the() << *ranges[i].ranges << "\n";
	}
}

DWARF::~DWARF(){
	
}

DWARFRange::DWARFRange(IntervalSet<uint32_t>* set, uint32_t offset, uint16_t ver){
	ranges = set;
	info_offset = offset;
	version = ver;
}

DWARFRange::DWARFRange(){

}

DWARFRange::~DWARFRange(){
	//delete ranges;
}
