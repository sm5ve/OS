#include <Scheduler.h>
#include <ds/Vector.h> //At some point we should consider using a better data structure

namespace Scheduler{
	Vector<Thread*>* threads;
	Thread* current = NULL;
	uint32_t index = 0;
	
	void init(){
		current = NULL;
		index = 0;
		threads = new Vector<Thread*>();
	}

	void addThread(Thread& t){
		threads -> push(&t);
	}

	void pickNext(){
		assert(threads -> size() > 0, "Error: scheduler empty");
		index = (index + 1) % (threads -> size());
		current = (*threads)[index];
	}
	
	void exec(){
		assert(current, "Error: no thread to switch to");
		current -> contextSwitch();
	}

	void storeState(registers& regs){
		if(current)
			current -> storeState(regs);
	}
	
	bool empty(){
		return threads -> size() == 0;
	}
}
