#include <klib/SerialDevice.h>

extern "C" void smp_init(){
	//for(;;);	
	SD::the() << "Hello from core 2!\n";
}
