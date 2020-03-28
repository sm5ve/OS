#ifndef INTERRUPTS
#define INTERRUPTS

class DisableInterrupts{
public:
	DisableInterrupts();
	~DisableInterrupts();
private:
	bool should_reenable;
};

#endif
