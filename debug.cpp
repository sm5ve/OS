#include <debug.h>
#include <klib/SerialDevice.h>

struct __attribute__((packed)) StackFrame{
	struct StackFrame* ebp;
	uint32_t eip;
};

void stackTrace(){
	StackFrame* frame;
	__asm__ volatile ("movl %%ebp, %0" : "r="(frame) ::);
	SD::the() << "Stack trace:\n";
	for(uint32_t i = 0; (frame != NULL) && (i < 16); i++){
		SD::the() << "\t" << (void*)(frame -> eip) << "\n";
		frame = frame -> ebp;
	}
}
