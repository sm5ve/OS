#include <interrupts.h>
#include <assert.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(__i386__)
#define X86
#endif

#ifdef X86
#include <arch/i386/proc.h> //TODO: Handle x86_64 case separately once we get 64-bit support
#endif

DisableInterrupts::DisableInterrupts(){
	#ifdef X86
	uint32_t eflags;
	asm("pushf\n" \
		"pop %%eax" : "=a"(eflags) ::);
	should_reenable = ((eflags & (1 << 9)) != 0);
	cli();
	#else	
	assert(false, "Attempted to disable interrupts on unknown CPU architecture");
	#endif
}

DisableInterrupts::~DisableInterrupts(){
	if(should_reenable){
		#ifdef X86
		sti();
		#else
		assert(false, "Attempted to reenable interrupts on unknown CPU architecture");
		#endif
	}
}
