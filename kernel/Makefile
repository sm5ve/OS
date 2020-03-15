OBJS := debug.o \
		init.o \
		interrupts.o \
		panic.o \
		arch/i386/higherHalf.o \
		arch/i386/interrupts.o \
		arch/i386/multiboot1.ao \
		arch/i386/proc.o \
		arch/i386/segments.o \
		drivers/pci.o \
		mem/Alloc.o \
		mem/BumpAlloc.o \
		mem/HeapAlloc.o \
		mem/SlabAlloc.o \
		mem/util.o \
		mem/paging/PageDirectory.o \
		mem/paging/Palloc.o \
		klib/CPPCompat.o \
		../lib/LibCore/PrintStream.o \
		klib/SerialDevice.o \
		../lib/LibCore/ds/String.o \
		../lib/LibCore/util/str.o \
		../lib/LibELF/dwarf.o \
		../lib/LibELF/elf.o

CC := i686-elf-gcc

INCLUDES := -I./include -I../lib/LibELF/include -I../lib/LibCore/include

CPPFLAGS := -ffreestanding -fno-exceptions -fno-rtti -fno-sized-deallocation -std=gnu++2a -g
CPPFLAGS += -Wall -Wextra -pedantic -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Winline -Wno-long-long -Wconversion
CPPFLAGS += $(INCLUDES)

ASMFLAGS := -ffreestanding -g
ASMFLAGS += $(INCLUDES)

.PHONY: clean kernel kernel.sym term
.DEFAULT_GOAL := term

kernel: $(OBJS)
	$(CC) -T linker.ld -o kernel -ffreestanding -nostdlib $(OBJS) -lgcc -g

kernel.sym: kernel
	@objcopy --only-keep-debug kernel kernel.sym
	@objcopy --strip-debug kernel

%.o: %.cpp Makefile
	@$(CC) $(CPPFLAGS) -c $< -o $@

%.ao: %.sx Makefile
	@$(CC) $(ASMFLAGS) -c $< -o $@

clean: 
	-@$(RM) $(wildcard $(OBJS) kernel kernel.sym)

term: kernel kernel.sym
	@qemu-system-x86_64 -m 1G -kernel kernel -initrd "kernel.sym" -nographic -no-reboot -d cpu_reset
