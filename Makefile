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
CPPFLAGS ?= 

INCLUDE=-I./include

.PHONY: all clean run kernel term

WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-declarations \
            -Wredundant-decls -Winline -Wno-long-long \
            -Wconversion

CPPFLAGS := -ffreestanding

all: kernel

kernel: $(OBJFILES)
	$(CC) -T linker.ld -o kernel -ffreestanding -nostdlib $(OBJFILES) -lgcc

-include $(DEPFILES)

%.cppo: %.cpp Makefile
	@$(CC) $(CPPFLAGS) $(INCLUDE) $(WARNINGS) -c $< -o $@

%.sxo: %.sx Makefile
	@$(CC) $(INCLUDE) -ffreestanding -c $< -o $@

run: kernel
	@qemu-system-x86_64 -serial stdio -kernel kernel

term: kernel
	@qemu-system-x86_64 -nographic -kernel kernel

clean:
	-@$(RM) $(wildcard $(OBJFILES) kernel)
