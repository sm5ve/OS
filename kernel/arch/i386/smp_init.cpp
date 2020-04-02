#include <klib/SerialDevice.h>
#include <devices/apic.h>

extern "C" void smp_init(){
	//for(;;);	
	SD::the() << "Hello from core " << (uint32_t)APIC::getLAPICID() << "!\n";
}
