#include <arch/i386/smp.h>
#include <devices/apic.h>
#include <devices/pit.h>

namespace SMP{
	void init(){
		uint32_t* apic = (uint32_t*)APIC::getLAPICAddr();
	}
}
