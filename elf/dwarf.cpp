#include <elf/dwarf.h>
#include <elf/dwarf_consts.h>
#include <klib/SerialDevice.h>

uint32_t decodeULEB128(void*& ptr){
	uint32_t out = 0;
	for(int i = 0;; i++){
		assert(i < 5, "Error: attempted to decode ULEB128 integer larger than uint32_t");
		uint8_t byte = *(char*)ptr;
		ptr = (void*)((uint32_t)ptr + 1);
		out |= (byte & 0x7f) << (7 * i);
		if((byte & 0x80) == 0){
			break;
		}
	}
	return out;
}

int decodeSLEB128(void*& ptr){
	int out = 0;
	int shift = 0;
	uint8_t byte;
	while(true){
		assert(shift < 32, "Error: attempted to decode SLEB128 integer larger than int");
		byte = *(char*)ptr;
		ptr = (void*)((uint32_t)ptr + 1);
		out |= (byte & 0x7f) << shift;
		shift += 7;
		if((byte & 0x80) == 0){
			break;
		}
	}
	if((shift < 32) && ((byte & 0x40) != 0)){
		out |= (~0 << shift);
	}
	return out;
}

DWARF::DWARF(ELF* e){
	elf = e;
	auto aranges = elf -> getSectionHeader(".debug_aranges");
	assert(aranges != NULL, "Error: missing aranges");
	void* ptr = elf -> getSectionBase(aranges);
	uint32_t end = (uint32_t)ptr + aranges -> size;
	while((uint32_t)ptr < end){
		DWARF_aranges_header header = *(DWARF_aranges_header*)ptr;
		assert(header.addr_size == 4, "Error: don't know how to handle differently sized addresses");
		assert(header.seg_size == 0, "Error: don't know how to handle segments");
		uint32_t* addrs = (uint32_t*)((uint32_t)ptr + sizeof(DWARF_aranges_header) + 4);
		ptr = (void*)((uint32_t)ptr + header.length + 4);
		auto set = new IntervalSet<uint32_t>();
		for(uint32_t i = 0; (addrs[i] != 0) || (addrs[i + 1] != 0); i += 2){
			set -> add(Interval<uint32_t>(addrs[i], addrs[i] + addrs[i + 1] - 1));
		}
		ranges.push(DWARFRange(set, header.info_offset, header.version));
	}
}

DWARF::~DWARF(){
	
}

uint32_t DWARF::getCUOffsetForAddr(void* ptr){
	for(uint32_t i = 0; i < ranges.size(); i++){
		if(ranges[i].ranges -> in((uint32_t)ptr)){
			return ranges[i].info_offset;
		}
	}
	return CU_NOT_FOUND;
}

Tuple<uint32_t, char*> DWARF::getLineForAddr(void* ptr){
	SD::the() << "Addr " << ptr << "\n";	
	uint32_t cuOffset = getCUOffsetForAddr(ptr);
	SD::the() << "CU offset " << (void*)cuOffset << "\n";
	auto info = elf -> getSectionHeader(".debug_info");
	void* sectionStart = elf -> getSectionBase(info);
	void* cuStart = (void*)((uint32_t)sectionStart + cuOffset);
	DWARF_info_header* header = (DWARF_info_header*)cuStart;
	SD::the() << "CU length     " << (void*)(header -> length) << "\n";
	SD::the() << "CU version    " << header -> version << "\n";
	SD::the() << "Abbrev offset " << (void*)(header -> abbrev_offset) << "\n";
	SD::the() << "Addr size     " << header -> addr_size << "\n";

	assert(header -> version == 4, "Error: don't know how to parse CU's with version != 4");
	assert(header -> addr_size == 4, "Error: don't know how to handle differently sized addresses");

	auto abbrevs = decodeAbbrev(header -> abbrev_offset);
	void* p = (void*)((uint32_t)header + sizeof(DWARF_info_header));

	void* p_backup = p;
	uint32_t name = decodeULEB128(p_backup);
	assert(abbrevs -> contains(name) && (abbrevs -> get(name) -> tag_type == DW_TAG_compile_unit), "Error: somehow CU doesn't start with compilation unit");

	DWARFDIE die(p, abbrevs, elf);

	SD::the() << "Source file " << (String*)die.value(DW_AT_name) << "\n";
	SD::the() << "Statment list offset " << (void*)*(uint32_t*)die.value(DW_AT_stmt_list) << "\n";
	
	//TODO we have to delete the DWARFSchemas too.
	//Or perhaps just make a new memory allocator for this - that's probably a better solution
	delete abbrevs;

	return Tuple<uint32_t, char*>(0, NULL);
}

DWARFSchema* tryParseDWARFSchema(void*& ptr){
	void* p = ptr;
	uint32_t name = decodeULEB128(p);
	void* pt = p;
	uint32_t type = decodeULEB128(pt);
	if(name == 0){
		ptr = pt;
		return NULL;
	}
	return new DWARFSchema(ptr);
}

HashMap<uint32_t, DWARFSchema*>* DWARF::decodeAbbrev(uint32_t offset){
	auto abbrevHeader = elf -> getSectionHeader(".debug_abbrev");
	void* sectionStart = elf -> getSectionBase(abbrevHeader);
	void* ptr = (void*)((uint32_t)sectionStart + offset);
	
	auto list = new HashMap<uint32_t, DWARFSchema*>();
	while(true){
		DWARFSchema* schema = tryParseDWARFSchema(ptr);
		if(schema == NULL){
			break;
		}
		list -> put(schema -> abbrev_code, schema);
	}
	return list;
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

DWARFSchema::DWARFSchema(void*& ptr){
	abbrev_code = decodeULEB128(ptr);
	tag_type = decodeULEB128(ptr);
	char c = *(char*)ptr;
	ptr = (void*)((uint32_t)ptr + 1);
	if(c == DW_CHILDREN_no){
		has_children = false;
	}
	else if(c == DW_CHILDREN_yes){
		has_children = true;
	}
	else{
		assert(false, "Error: malformed DWARF schema");
	}
	while(true){
		uint32_t name = decodeULEB128(ptr);
		uint32_t form = decodeULEB128(ptr);
		if((name == 0) && (form == 0)){
			break;
		}
		fields.push(Tuple<uint32_t, uint32_t>(name, form));
	}
}

String* getDWARFString(uint32_t offset, ELF* elf){
	auto header = elf -> getSectionHeader(".debug_str");
	void* ptr = elf -> getSectionBase(header);
	char* c_str = (char*)((uint32_t)ptr + offset);
	return new String(c_str);
}

void* parseDIETag(void*& ptr, uint32_t type, ELF* elf){
	switch(type){
		case DW_FORM_addr:
			{
				void** addr = new void*;
				*addr = (void*)*(uint32_t*)ptr;
				ptr = (void*)((uint32_t)ptr + 4);
				return addr;
			}
		case DW_FORM_data2:
			{
				uint64_t* val = new uint64_t;
				*val = (uint64_t)*(uint16_t*)ptr;
				ptr = (void*)((uint32_t)ptr + 2);
				return val;
			}
		case DW_FORM_data4:
			{
				uint64_t* val = new uint64_t;
				*val = (uint64_t)*(uint32_t*)ptr;
				ptr = (void*)((uint32_t)ptr + 4);
				return val;
			}
		case DW_FORM_data8:
			{
				uint64_t* val = new uint64_t;
				*val = *(uint64_t*)ptr;
				ptr = (void*)((uint32_t)ptr + 8);
				return val;
			}
		case DW_FORM_data1:
			{
				uint64_t* val = new uint64_t;
				*val = (uint64_t)*(uint8_t*)ptr;
				ptr = (void*)((uint32_t)ptr + 1);
				return val;
			}
		case DW_FORM_strp:
			{ 
				uint32_t offset = *(uint32_t*)ptr; 
				ptr = (void*)((uint32_t)ptr + 4); 
				return getDWARFString(offset, elf);
			}
		case DW_FORM_sec_offset:
			{
				uint32_t* out = new uint32_t;
				*out = *(uint32_t*)ptr;
				ptr = (void*)((uint32_t)ptr + 4);
				return out;
			}
		default: assert(false, "Unimplemented DWARF tag type");
	}
}

DWARFDIE::DWARFDIE(void*& ptr, HashMap<uint32_t, DWARFSchema*>* schemas, ELF* elf){
	uint32_t abbrev_type = decodeULEB128(ptr);
	assert(schemas -> contains(abbrev_type), "Error: unknown schema type");
	DWARFSchema* schema = schemas -> get(abbrev_type);
	for(int i = 0; i < schema -> fields.size(); i++){
		auto field = schema -> fields[i];
		SD::the() << "name " << field.a << " type " << field.b << "\n";
		map.put(field.a, parseDIETag(ptr, field.b, elf));
	}
}

void* DWARFDIE::value(uint32_t name){
	if(map.contains(name)){
		return map.get(name);
	}
	return NULL;
}
