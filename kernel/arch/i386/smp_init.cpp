#include <arch/i386/smp.h>
#include <devices/SerialDevice.h>
#include <devices/apic.h>

extern "C" void smp_init()
{
	installGDT();
	SMP::incCoreCount();
	SD::the() << "Hello from core " << (uint32_t)APIC::getLAPICID() << "!\n";
}
