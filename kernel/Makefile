OBJS := debug.o \
		init.o \
		interrupts.o \
		panic.o \
		acpi/tables.o \
		arch/i386/higherHalf.o \
		arch/i386/interrupts.o \
		arch/i386/multiboot1.ao \
		arch/i386/context_switch.ao \
		arch/i386/proc.o \
		arch/i386/segments.o \
		arch/i386/smp.o \
		arch/i386/smp_boot.ao \
		arch/i386/smp_init.o \
		devices/apic.o \
		devices/ahci/ahci.o \
		devices/ahci/AHCIHostBusAdapter.o \
		devices/pci.o \
		devices/pcie.o \
		devices/PCIDevice.o \
		devices/pit.o \
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
		scheduler/Scheduler.o \
		scheduler/Task.o \
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

QEMUFLAGS := -m 1G -nographic -no-reboot -device VGA,vgamem_mb=64 -machine q35 
QEMUFLAGS += -drive id=disk,file=../hd.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.2
QEMUFLAGS += -smp 4
#QEMUFLAGS += -d cpu_reset
#QEMUFLAGS += -d int
#I'm playing around with GVT-g. Perhaps one day I'll get it working and i915 support will be a little more realistic
#QEMUFLAGS += -enable-kvm 
#QEMUFLAGS += -device vfio-pci,sysfsdev=/sys/devices/pci0000:00/0000:00:02.0/4f61c2c2-71ad-11ea-878d-775857b37de3,x-igd-opregion=on,display=on


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
	@qemu-system-x86_64 $(QEMUFLAGS) -kernel kernel -initrd "kernel.sym,test"
