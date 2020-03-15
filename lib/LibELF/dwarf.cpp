#include <elf/dwarf.h>
#include <elf/dwarf_consts.h>
#include <util/str.h>
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

Tuple<uint32_t, String> DWARF::getLineForAddr(void* ptr){
	uint32_t cuOffset = getCUOffsetForAddr(ptr);
	if(cuOffset == CU_NOT_FOUND){
		return Tuple<uint32_t, String>(0, "?");
	}
	auto info = elf -> getSectionHeader(".debug_info");
	void* sectionStart = elf -> getSectionBase(info);
	void* cuStart = (void*)((uint32_t)sectionStart + cuOffset);
	DWARF_info_header* header = (DWARF_info_header*)cuStart;
	//SD::the() << header -> version << "\n";
	//assert(header -> version == 4, "Error: don't know how to parse CU's with version != 4");
	assert(header -> addr_size == 4, "Error: don't know how to handle differently sized addresses");

	auto abbrevs = decodeAbbrev(header -> abbrev_offset);
	void* p = (void*)((uint32_t)header + sizeof(DWARF_info_header));

	void* p_backup = p;
	uint32_t name = decodeULEB128(p_backup);
	assert(abbrevs -> contains(name) && (abbrevs -> get(name) -> tag_type == DW_TAG_compile_unit), "Error: somehow CU doesn't start with compilation unit");

	DWARFDIE die(p, abbrevs, elf);
	DWARFLineStateMachine sm(*(uint32_t*)die.value(DW_AT_stmt_list), elf);
	
	//TODO we have to delete the DWARFSchemas too.
	//Or perhaps just make a new memory allocator for this - that's probably a better solution
	delete abbrevs;
	
	auto result = sm.getLineForAddr(ptr);
	if(result.has_value())
		return result.value();
	return Tuple<uint32_t, String>(0, "?");
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
		map.put(field.a, parseDIETag(ptr, field.b, elf));
	}
}

void* DWARFDIE::value(uint32_t name){
	if(map.contains(name)){
		return map.get(name);
	}
	return NULL;
}

DWARFLineStateMachine::DWARFLineStateMachine(uint32_t index, ELF* e){
	auto lines_section_header = e -> getSectionHeader(".debug_line");
	void* ptr = e -> getSectionBase(lines_section_header);
	ptr = (void*)((uint32_t)ptr + index);
	DWARF_line_header* header = (DWARF_line_header*)ptr;
	assert(header -> opcode_base == 13, "Error: don't know how to deal with nonstandard opcodes"); //FIXME this is kind of a hack
	
	statements_start = (void*)((uint32_t)&(header -> min_instruction_length) + (header -> header_length));
	section_end = (void*)((uint32_t)&(header -> version) + (header -> length));
	default_is_stmt = (header -> default_is_stmt != 0);
	min_inst_len = header -> min_instruction_length; 
	line_base = header -> line_base;
	line_range = header -> line_range;
	opcode_base = header -> opcode_base;

	ptr = (void*)((uint32_t)ptr + sizeof(DWARF_line_header));
	while(strlen((char*)ptr) > 0){
		directories.push((char*)ptr);
		ptr = (void*)((uint32_t)ptr + strlen((char*)ptr) + 1);
	}
	ptr = (void*)((uint32_t)ptr + 1);
	while(strlen((char*)ptr) > 0){
		char* name = (char*)ptr;
		ptr = (void*)((uint32_t)ptr + strlen((char*)ptr) + 1);
		uint32_t dir = decodeULEB128(ptr);
		uint32_t last_modified = decodeULEB128(ptr);
		uint32_t file_size = decodeULEB128(ptr);
		files.push({name, dir, last_modified, file_size});
	}
}

void DWARFLineStateMachine::reset(){
	addr = 0;
	op_index = 0;
	file = 1;
	line = 1;
	col = 0;
	is_stmt = default_is_stmt;
	basic_block = false;
	end_seq = false;
	prologue_end = false;
	epilogue_begin = false;
	isa = 0;
	discriminator = 0;
	did_copy = false;
	did_special = false;
	just_ended_sequence = false;
}

//TODO why are we making so many Maybe<Interval>s? This happens elsewhere in this file.
Maybe<Tuple<uint32_t, String>> DWARFLineStateMachine::getLineForAddr(void* ptr){
	reset();
	void* ip = statements_start;	
	uint32_t last_line;
	uint32_t last_file;
	uint32_t last_addr;
	while(ip < section_end){
		if(step(ip)){
			if((addr == (uint32_t)ptr) || ((last_addr < (uint32_t)ptr) && (addr > (uint32_t) ptr))){
				auto fe = files[last_file - 1];
				String fname(fe.name);
				String dir = ".";
				if(fe.directory != 0){
					dir = String(directories[fe.directory - 1]);
				
				}
				String path = dir + "/" + fname;
				Tuple<uint32_t, String> val = Tuple<uint32_t, String>(last_line, path);
				Maybe<Tuple<uint32_t, String>> out(val);
				return out;
			}
			last_line = line;
			last_file = file;
			last_addr = addr;
		}
	}
	return Maybe<Tuple<uint32_t, String>>();
}

bool DWARFLineStateMachine::step(void*& ip){
	uint8_t opcode = *(uint8_t*)ip;
	ip = (void*)((uint32_t)ip + 1);
	if(did_copy || did_special){
		discriminator = 0;
		basic_block = false;
		prologue_end = false;
		epilogue_begin = false;
		did_copy = false;
		did_special = false;
	}
	if(just_ended_sequence){
		reset();
	}
	if(1 <= opcode && opcode <= 12){
		switch(opcode){
			case DW_LNS_copy: did_copy = true; return true;
			case DW_LNS_advance_pc:
			{
				uint32_t inc = decodeULEB128(ip);
				addr += inc * min_inst_len;
			} break;
			case DW_LNS_advance_line:
				line += decodeSLEB128(ip);
				break;
			case DW_LNS_set_file:
				file = decodeULEB128(ip);
				break;
			case DW_LNS_set_column:
				col = decodeULEB128(ip);
				break;
			case DW_LNS_negate_stmt:
				is_stmt = !is_stmt;
				break;
			case DW_LNS_set_basic_block:
				basic_block = true;
				break;
			case DW_LNS_const_add_pc:
				addr += min_inst_len * ((255 - opcode_base) / line_range);
				break;
			case DW_LNS_fixed_advance_pc:
			{
				uint16_t adv = *(uint16_t*)ip;
				ip = (void*)((uint32_t)ip + 2);
				addr += adv;
			} break;
			case DW_LNS_set_prologue_end:
				prologue_end = true;
				break;
			case DW_LNS_set_epilogue_begin:
				epilogue_begin = true;
				break;
			case DW_LNS_set_isa:
				isa = decodeULEB128(ip);
				break;
		}
		return false;
	}
	else if(opcode == 0){
		uint8_t opcode_size = *(uint8_t*)ip;
		ip = (void*)((uint32_t)ip + 1);
		uint8_t ext_opcode = *(uint8_t*)ip;
		ip = (void*)((uint32_t)ip + 1);
		switch(ext_opcode){
			case DW_LNE_end_sequence:
				end_seq = true;
				just_ended_sequence = true;
				return true;
			case DW_LNE_set_address:
				addr = *(uint32_t*)ip;
				ip = (void*)((uint32_t)ip + 4);
				break;
			case DW_LNE_define_file:
				assert(false, "Unimplemented");
			case DW_LNE_set_discriminator:
				discriminator = decodeULEB128(ip);
				break;
			default:
				SD::the() << "unknown extended opcode " << ext_opcode << "\n";
				assert(false, "WUT");
		}
		return false;
	}
	else{
		uint8_t special_opcode = opcode - opcode_base;
		addr += (special_opcode / line_range) * min_inst_len;
		line += line_base + (special_opcode % line_range);
		did_special = true;
		return true;
	}
	return false;
}
