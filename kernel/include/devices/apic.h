#ifndef APIC_hdr
#define APIC_hdr

#include <stdint.h>
#include <ds/Vector.h>
#include <ds/Tuple.h>

struct __attribute__((packed)) APIC_local_descriptor{
	uint32_t local_APIC_addr;
	uint32_t flags;
};

struct __attribute__((packed)) MDAT_entry_header{
	uint8_t entry_type;
	uint8_t length;
};

struct __attribute__((packed)) MDAT_local_APIC_entry{
	uint8_t APIC_processor_id;
	uint8_t APIC_id;
	uint32_t flags;
};

struct __attribute__((packed)) MDAT_io_APIC_entry{
	uint8_t IO_APIC_id;
	uint8_t reserved;
	uint32_t IO_APIC_addr;
	uint32_t global_system_interrupt_base;
};

struct __attribute__((packed)) MDAT_interrupt_source_override_entry{
	uint8_t bus_source;
	uint8_t irq_source;
	uint32_t global_system_interrupt;
	uint16_t flags;
};

struct __attribute__((packed)) MDAT_NMI_entry{
	uint8_t APIC_processor_id;
	uint16_t flags;
	uint8_t LINT_num;
};

struct __attribute__((packed)) MDAT_local_APIC_address_override_entry{
	uint16_t reserved;
	uint64_t paddr;
};

namespace APIC{
	void init();
	void* getLAPICAddr();
	uint8_t getLAPICID();
	Vector<Tuple<uint8_t, uint8_t>>* getCoresWithAPICIDs();
}

#endif
