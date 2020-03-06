#include <debug.h>
#include <klib/SerialDevice.h>

DWARF* ksyms = NULL;

struct __attribute__((packed)) StackFrame{
	struct StackFrame* ebp;
	uint32_t eip;
};

void stackTrace(uint32_t skip){
	if(ksyms){
		prettyStackTrace(skip);
	}
	else{
		plainStackTrace(skip);
	}
}

void plainStackTrace(uint32_t skip){
	StackFrame* frame;
	__asm__ volatile ("movl %%ebp, %0" : "=r"(frame) ::);
	SD::the() << "Stack trace:\n";
	for(uint32_t i = 0; (frame != NULL) && (i < 16); i++){
		if(i >= skip)
			SD::the() << "\t" << (void*)(frame -> eip) << "\n";
		frame = frame -> ebp;
	}
}

void prettyStackTrace(uint32_t skip){
	StackFrame* frame;
	__asm__ volatile ("movl %%ebp, %0" : "=r"(frame) ::);
	SD::the() << "Stack trace:\n";
	for(uint32_t i = 0; (frame != NULL) && (i < 16); i++){
		if(i >= skip){
			auto symbolicated = ksyms -> getLineForAddr((void*)(frame -> eip));
			SD::the() << "\t[" << (void*)(frame -> eip) << "]: " << symbolicated.b << " line " << symbolicated.a << "\n";
		}
		frame = frame -> ebp;
	}
}
