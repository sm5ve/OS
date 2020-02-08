#include <arch/i386/proc.h>

void outb(uint16_t port, uint8_t out){
	__asm__ volatile("outb %0, %1" :: "a" (out),  "Nd" (port));
}

uint8_t inb(uint16_t port){
	uint8_t out;
	__asm__ volatile("inb %1, %0": "=a" (out) : "Nd" (port));
	return out;
}

void outw(uint16_t port, uint16_t word){
	__asm__ volatile("outw %0, %1" :: "a" (word), "Nd" (port));
}

uint16_t inw(uint16_t port){
	uint16_t out;
	__asm__ volatile("inw %1, %0": "=a" (out) : "Nd" (port));
	return out;
}

void cli(){
	__asm__ volatile("cli");
}

void sti(){
	__asm__ volatile("sti");
}
