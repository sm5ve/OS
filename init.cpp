#include <multiboot/multiboot.h>
#include <klib/SerialPrinter.h>

extern "C" [[noreturn]] void kernel_init(unsigned int multiboot2_magic, unsigned long addr){
	SerialPrinter p(COMPort::COM1);
	p << "\ntest\n" << "this is pretty cool!";
	for(int i = 0; i < 10; i++){
		p << i << "\n";
	}
	p << (void*)&p;
	for(;;);
}
