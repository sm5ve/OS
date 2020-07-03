# Unnamed OS

The goal of this project is to create a basic POSIX-ish kernel capable of running basic POSIX-y programs. Eventually, it would be fun to create a nice GUI with some hardware accelerated graphics, but my understanding is that unchecked delusions of writing a graphics card driver are very dangerous.

So far, the kernel has support for
* Output via the serial console
* Pretty-printed stack traces (with line numbers!) by parsing DWARF sections
* Loading ELF files passed in through multiboot
* Ring 3 code execution
* A basic round-robin task scheduler.
* Preliminary SMP support (I can spin up the extra cores, but they don't do anything yet)

My immediate goals are:

* Update data structures to support range-based for (in-progress)
  * Update kernel to actually use range-based for
* Work on commenting the system
* Code review
* Finally implement the command queuing system for AHCI that I'd meant to do

Features/subsystems I hope to implement in the near future include (in roughly the order of priority)

* AHCI driver
* GPT partition table driver
* EXT2 filesystem driver
* A VFS
* A proper threading/process model
* A basic userspace
* A name
* A more generic driver framework

In the longer term, I would like to support the following features

* A TCP/IP networking stack of some sort
* Perhaps a desktop of some sort (As per request of my friend, the shell will be named GNOBLIN shell)
* Doom (As per request of my brother)
* Some sort of support for a version of intel integrated graphics.
