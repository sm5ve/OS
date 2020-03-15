#include <interrupts.h>
#include <assert.h>

#if defined(__x86_64__) || defined(__i386__)
#define X86
#endif

#ifdef X86
#include <arch/i386/proc.h> //TODO: Handle x86_64 case separately once we get 64-bit support
#endif

DisableInterrupts::DisableInterrupts(){
	#ifdef X86
	cli();
	#else	
	assert(false, "Attempted to disable interrupts on unknown CPU architecture");
	#endif
}

DisableInterrupts::~DisableInterrupts(){
	#ifdef X86
	sti();
	#else
	assert(false, "Attempted to reenable interrupts on unknown CPU architecture");
	#endif
}
