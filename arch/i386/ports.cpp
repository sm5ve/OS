#include <arch/i386/proc.h>

void outb(unsigned short port, unsigned char out){
	__asm__ volatile("outb %0, %1" :: "a" (out),  "Nd" (port));
}

unsigned char inb(unsigned short port){
	unsigned char out;
	__asm__ volatile("inb %1, %0": "=a" (out) : "Nd" (port));
	return out;
}
