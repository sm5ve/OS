#ifndef SCHEDULER
#define SCHEDULER
#include <Thread.h>
#include <arch/i386/proc.h>

namespace Scheduler{
	void init();
	void addThread(Thread&);
	void removeThread(Thread&);
	void pickNext();
	void exec();
	void storeState(registers&);
	bool empty();
}

#endif
