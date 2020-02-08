#ifndef I386_HEADER
#define I386_HEADER

#include <stdint.h>

void outb(uint16_t port, uint8_t c);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t w);
uint16_t inw(uint16_t port);


void cli();
void sti();

#endif
