PROJDIRS := .

CPPFILES := $(shell find $(PROJDIRS) -type f -name "*.cpp")
ASMFILES := $(shell find $(PROJDIRS) -type f -name "*.sx")
HDRFILES := $(shell find $(PROJDIRS) -type f -name "*.h")

SRCFILES := $(CPPFILES) $(ASMFILES)

CPPOBJFILES := $(patsubst %.cpp,%.cppo,$(CPPFILES))
ASMOBJFILES := $(patsubst %.sx,%.sxo,$(ASMFILES))

OBJFILES := $(CPPOBJFILES) $(ASMOBJFILES)

DEPFILES    := $(patsubst %.cpp,%.d,$(CPPFILES))

CC := i686-elf-gcc
NM := i686-elf-nm
CPPFLAGS ?= 

INCLUDE=-I./include

.PHONY: all clean run kernel term

WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wcast-align \
            -Wwrite-strings \
            -Wredundant-decls -Winline -Wno-long-long \
            -Wconversion

CPPFLAGS := -ffreestanding -fno-exceptions -fno-rtti

all: kernel

kernel: $(OBJFILES)
	$(CC) -T linker.ld -o kernel -ffreestanding -nostdlib $(OBJFILES) -lgcc -g

-include $(DEPFILES)

%.cppo: %.cpp Makefile
	@$(CC) $(CPPFLAGS) $(INCLUDE) $(WARNINGS) -fno-sized-deallocation -std=gnu++2a -c $< -o $@ -g

%.sxo: %.sx Makefile
	@$(CC) $(INCLUDE) -ffreestanding -c $< -o $@ -g

run: kernel
	@objcopy --only-keep-debug kernel kernel.sym
	@objcopy --strip-debug kernel
	@qemu-system-x86_64 -m 1G -serial stdio -kernel kernel -initrd "kernel.sym"

term: kernel
	@objcopy --only-keep-debug kernel kernel.sym
	@objcopy --strip-debug kernel
	@qemu-system-x86_64 -m 1G -nographic -no-reboot -d cpu_reset -kernel kernel -initrd "kernel.sym"

ints: kernel
	@qemu-system-x86_64 -m 1G -nographic -d int,cpu_reset -no-reboot -kernel kernel

clean:
	-@$(RM) $(wildcard $(OBJFILES) kernel kernel.sym)
