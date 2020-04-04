#include <devices/pit.h>
#include <arch/i386/proc.h>

#define PIT_CHAN0 0x40
#define PIT_CHAN1 0x41
#define PIT_CHAN2 0x42
#define PIT_MODE  0x43

#define PIT_FREQ 1193182

namespace PIT{
	uint64_t wait_ticks = 0;
	volatile bool waiting;
	void waitMaxRemainingTicks();
	InterruptHandlerDecision handleIRQ(registers&){
		if(wait_ticks == 0){
			waiting = false;
		}
		else{
			waitMaxRemainingTicks();
		}
		outb(PIT_MODE, 0x30);
		return InterruptHandlerDecision::CONSUME;
	}
	
	void initOneshot(){
		outb(PIT_MODE, 0x30);
		IDT::installIRQHandler(handleIRQ, 0);
	}

	void waitTicks(uint16_t ticks){
		outb(PIT_CHAN0, ticks & 0xff);
		outb(PIT_CHAN0, (ticks >> 8) & 0xff);
	}

	void waitMaxRemainingTicks(){
		if(wait_ticks > 0xffff){
			waitTicks(0xffff);
			wait_ticks -= 0xffff;
		}
		else{
			waitTicks((uint16_t)wait_ticks);
			wait_ticks = 0;
		}
	}
	//Note: this does NOT support multiple timers, and may very well be a temporary solution while we get SMP up and running
	void waitMicros(uint32_t us){
		uint64_t ticks = us;
		ticks *= PIT_FREQ;
		ticks /= 1000000;
		wait_ticks = ticks;
		waitMaxRemainingTicks();
		waiting = true;
		while(waiting);
	}
	
	void waitMillis(uint32_t ms){
		waitMicros(ms * 1000);
	}
}
