#ifndef SMP_header
#define SMP_header

#include <stdint.h>

namespace SMP{
	enum IPIMode{
		NORMAL = 0,
		LOW = 1,
		SMI = 2,
		NMI = 4,
		INIT = 5,
		SIPI = 6
	};
	
	void setupAPTables(uint16_t offset);
	void init();
	void sendIPI(uint8_t apic_id, uint8_t vector, IPIMode mode);
}

#endif
