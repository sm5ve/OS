OBJS := debug.o \
		init.o \
		interrupts.o \
		panic.o \
		arch/i386/higherHalf.o \
		arch/i386/interrupts.o \
		arch/i386/multiboot1.ao \
		arch/i386/proc.o \
		arch/i386/segments.o \
		devices/pci.o \
		loader/loader.o \
		mem/Alloc.o \
		mem/BumpAlloc.o \
		mem/HeapAlloc.o \
		mem/SlabAlloc.o \
		mem/util.o \
		mem/paging/BootstrapPaging.o\
		mem/paging/MemoryManager.o \
		mem/paging/PageDirectory.o \
		mem/paging/MemoryRegionReference.o \
		mem/paging/MemoryRegion.o \
		scheduler/Process.o \
		klib/CPPCompat.o \
		klib/SerialDevice.o \
		../lib/LibCore/PrintStream.o \
		../lib/LibCore/ds/String.o \
		../lib/LibCore/util/str.o \
		../lib/LibELF/dwarf.o \
		../lib/LibELF/elf.o

CC := i686-elf-gcc

INCLUDES := -I./include -I../lib/LibELF/include -I../lib/LibCore/include

CPPFLAGS := -ffreestanding -fno-exceptions -fno-rtti -fno-sized-deallocation -std=gnu++2a -g -fconcepts
CPPFLAGS += -Wall -Wextra -pedantic -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Winline -Wno-long-long -Wconversion
CPPFLAGS += $(INCLUDES)

ASMFLAGS := -ffreestanding -g
ASMFLAGS += $(INCLUDES)

.PHONY: clean kernel kernel.sym term
.DEFAULT_GOAL := term

test: test_program/main.cpp
	$(CC) -ffreestanding -nostdlib -lgcc -fno-exceptions $< -o test

kernel: $(OBJS)
	$(CC) -T linker.ld -o kernel -ffreestanding -nostdlib $(OBJS) -lgcc -g

kernel.sym: kernel
	@i686-elf-objcopy --only-keep-debug kernel kernel.sym
	@i686-elf-objcopy --strip-debug kernel

%.o: %.cpp Makefile
	@$(CC) $(CPPFLAGS) -c $< -o $@

%.ao: %.sx Makefile
	@$(CC) $(ASMFLAGS) -c $< -o $@

clean: 
	-@$(RM) $(wildcard $(OBJS) kernel kernel.sym test)

term: kernel kernel.sym test
	@qemu-system-x86_64 -m 1G -kernel kernel -initrd "kernel.sym,test" -nographic -no-reboot -d cpu_reset
