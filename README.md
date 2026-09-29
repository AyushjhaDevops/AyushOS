# AyushOS

> **A research-oriented operating system built from scratch to explore kernel architecture, CPU execution, interrupts, process management, scheduling, memory management, user-space isolation, and low-level systems engineering.**

![Architecture](https://img.shields.io/badge/Architecture-x86%2032--bit-informational)
![Kernel](https://img.shields.io/badge/Kernel-Experimental-orange)
![Language](https://img.shields.io/badge/Language-C-blue)
![Assembly](https://img.shields.io/badge/Assembly-NASM-red)
![Bootloader](https://img.shields.io/badge/Bootloader-GRUB-purple)
![Build](https://img.shields.io/badge/Build-Make-green)
![Emulator](https://img.shields.io/badge/Emulator-QEMU-yellow)
![Debugger](https://img.shields.io/badge/Debugger-GDB-blue)
![Status](https://img.shields.io/badge/Status-Active%20Development-orange)

---

## Overview

**AyushOS** is an experimental operating-system kernel developed from scratch for studying how an operating system interacts with the CPU, memory, hardware devices, and user programs.

The project starts at the bootloader/kernel boundary and progressively develops the major components required for a modern operating-system architecture.

The long-term goal is to transform AyushOS from a minimal educational kernel into a **research-oriented systems platform** for experimenting with:

* Kernel architecture
* CPU execution
* Protected mode
* Interrupt handling
* Hardware drivers
* Physical and virtual memory
* Process management
* CPU scheduling
* System calls
* User/kernel isolation
* Filesystems
* Networking
* Kernel security
* Multicore execution
* Operating-system research

AyushOS is **not intended to replace Linux or other production operating systems**.

Its purpose is to understand what happens underneath them.

---

# Project Philosophy

AyushOS follows a simple principle:

> **Build from the hardware upward. Understand every layer. Measure what you build.**

The project follows:

```text
Understand
    ↓
Design
    ↓
Implement
    ↓
Boot
    ↓
Test
    ↓
Debug
    ↓
Measure
    ↓
Document
    ↓
Research
```

Every major subsystem should eventually contain:

* Implementation
* Architecture documentation
* Tests or validation
* Debugging support
* Performance measurements
* Design notes

---

# High-Level Architecture

The long-term AyushOS architecture is:

```text
┌──────────────────────────────────────────────────────┐
│                    USER SPACE                        │
│                                                      │
│        Applications / Shell / Utilities              │
└─────────────────────────┬────────────────────────────┘
                          │
                          │ System Calls
                          ▼
┌──────────────────────────────────────────────────────┐
│                    KERNEL SPACE                      │
│                                                      │
│  ┌───────────────┐   ┌────────────────────────────┐  │
│  │ Process       │   │ System Call Interface      │  │
│  │ Management    │   └────────────────────────────┘  │
│  └───────┬───────┘                                  │
│          │                                            │
│  ┌───────▼───────┐   ┌────────────────────────────┐  │
│  │ Scheduler      │   │ Virtual Memory             │  │
│  └───────┬───────┘   └────────────────────────────┘  │
│          │                                            │
│  ┌───────▼───────┐   ┌────────────────────────────┐  │
│  │ Memory        │   │ Virtual File System         │  │
│  │ Manager       │   └────────────────────────────┘  │
│  └───────────────┘                                  │
│                                                      │
│  ┌────────────────────────────────────────────────┐  │
│  │              Hardware Drivers                  │  │
│  │ Timer │ Keyboard │ Storage │ Network │ Console  │  │
│  └───────────────────────┬────────────────────────┘  │
└──────────────────────────┼───────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────┐
│                    CPU / HARDWARE                    │
│                                                      │
│          x86 CPU • Memory • Devices • IRQ            │
└──────────────────────────────────────────────────────┘
```

---

# Current Kernel Architecture

The current implementation contains the foundational x86 kernel infrastructure:

```text
                         ┌──────────────────┐
                         │       GRUB       │
                         │    Bootloader    │
                         └────────┬─────────┘
                                  │
                                  ▼
                         ┌──────────────────┐
                         │   x86 Boot Code  │
                         │      NASM        │
                         └────────┬─────────┘
                                  │
                                  ▼
                         ┌──────────────────┐
                         │   Kernel Entry   │
                         │   kernel_main()  │
                         └────────┬─────────┘
                                  │
              ┌───────────────────┼───────────────────┐
              │                   │                   │
              ▼                   ▼                   ▼
        ┌──────────┐        ┌──────────┐        ┌──────────┐
        │   GDT    │        │   IDT    │        │   PIC    │
        │Segmentation│      │Interrupt │        │  IRQ     │
        └──────────┘        └────┬─────┘        └────┬─────┘
                                  │                   │
                                  └────────┬──────────┘
                                           ▼
                                  ┌────────────────┐
                                  │ IRQ Subsystem  │
                                  └───────┬────────┘
                                          │
                         ┌────────────────┼────────────────┐
                         │                │                │
                         ▼                ▼                ▼
                    ┌─────────┐      ┌──────────┐    ┌──────────┐
                    │  Timer  │      │ Keyboard │    │ Future   │
                    │  IRQ0   │      │   IRQ1   │    │ Devices  │
                    └────┬────┘      └─────┬────┘    └──────────┘
                         │                 │
                         └────────┬────────┘
                                  ▼
                         ┌────────────────────┐
                         │ Process Management │
                         │                    │
                         │ PID                │
                         │ Process State      │
                         │ Kernel Stack       │
                         │ CPU Context        │
                         └─────────┬──────────┘
                                   │
                                   ▼
                         ┌────────────────────┐
                         │ Context Switching  │
                         │       NASM         │
                         └─────────┬──────────┘
                                   │
                    ┌──────────────┼──────────────┐
                    ▼              ▼              ▼
                ┌────────┐    ┌────────┐     ┌────────┐
                │Process │    │Process │     │  Idle  │
                │   A    │    │   B    │     │Process │
                └────────┘    └────────┘     └────────┘
```

---

# Technology Stack

| Component           | Technology                 |
| ------------------- | -------------------------- |
| Target Architecture | x86 32-bit                 |
| Kernel              | Custom experimental kernel |
| Kernel Language     | C                          |
| Low-Level Assembly  | NASM                       |
| Bootloader          | GRUB                       |
| Compiler            | GCC                        |
| Linker              | GNU `ld`                   |
| Build System        | GNU Make                   |
| Emulator            | QEMU                       |
| Debugger            | GDB                        |
| Host OS             | Arch Linux                 |
| Version Control     | Git / GitHub               |

---

# Repository Structure

```text
AyushOS/
│
├── arch/
│   └── x86/
│       ├── boot.asm
│       ├── context_switch.asm
│       ├── gdt_flush.asm
│       ├── idt_flush.asm
│       ├── interrupts.asm
│       └── irq.asm
│
├── include/
│   └── kernel/
│       ├── gdt.h
│       ├── idt.h
│       ├── irq.h
│       ├── keyboard.h
│       ├── kprintf.h
│       ├── pic.h
│       ├── process.h
│       ├── terminal.h
│       └── timer.h
│
├── kernel/
│   ├── kernel.c
│   ├── gdt.c
│   ├── idt.c
│   ├── irq.c
│   ├── pic.c
│   ├── process.c
│   ├── timer.c
│   │
│   ├── drivers/
│   │   ├── keyboard.c
│   │   └── terminal.c
│   │
│   └── lib/
│       └── kprintf.c
│
├── scripts/
│   └── linker.ld
│
├── iso/
│   └── boot/
│
├── Makefile
├── README.md
└── .gitignore
```

---

# Development Roadmap

AyushOS is organized into incremental development phases.

```text
Phase 0
Development Environment
       ↓
Phase 1
Boot + Kernel Entry
       ↓
Phase 2
Terminal + Logging
       ↓
Phase 3
GDT
       ↓
Phase 4
IDT + Exceptions
       ↓
Phase 5
PIC + IRQ
       ↓
Phase 6
Timer + Keyboard
       ↓
Phase 7
Physical Memory
       ↓
Phase 8
Process Management
       ↓
Phase 9
Preemptive Scheduler
       ↓
Phase 10
User Mode + System Calls
       ↓
Phase 11+
Virtual Memory / Filesystem / Networking / Security / SMP
```

---

# Phase 0 — Development Environment

## Objective

Create a reproducible low-level operating-system development environment.

### Toolchain

```text
Arch Linux
   │
   ├── GCC
   ├── NASM
   ├── GNU Binutils
   ├── GRUB
   ├── Xorriso
   ├── QEMU
   ├── GDB
   └── GNU Make
```

### Deliverables

* Development environment
* Repository initialization
* Makefile
* Cross-layer build pipeline
* QEMU execution
* GDB debugging

### Architecture

```text
Developer
    │
    ▼
Source Code
    │
    ▼
Assembler / Compiler
    │
    ▼
Linker
    │
    ▼
Kernel Binary
    │
    ▼
GRUB ISO
    │
    ▼
QEMU
```

---

# Phase 1 — Bootloader and Kernel Entry

## Objective

Boot the machine and transfer execution from GRUB to the AyushOS kernel.

### Architecture

```text
BIOS / UEFI
     ↓
GRUB
     ↓
boot.asm
     ↓
Kernel Entry
     ↓
kernel_main()
```

### Components

* GRUB configuration
* Multiboot-compatible boot path
* Assembly bootstrap
* Kernel entry point
* Linker script
* Kernel binary
* Bootable ISO

### Success Criteria

```text
GRUB
  ↓
AyushOS
  ↓
Kernel executes successfully
```

---

# Phase 2 — Terminal and Kernel Logging

## Objective

Create a basic kernel output system.

### Architecture

```text
Kernel
   │
   ▼
kprintf()
   │
   ▼
Terminal Driver
   │
   ▼
VGA Text Buffer
```

### Features

* VGA text mode
* Character output
* String output
* Integer formatting
* Kernel logging
* Error messages
* Debug messages

### Example

```text
[ OK ] Terminal initialized
[ OK ] GDT initialized
[ OK ] IDT initialized
[ OK ] PIC initialized
[INFO] Kernel boot complete
```

---

# Phase 3 — Global Descriptor Table

## Objective

Establish x86 protected-mode segmentation.

### Architecture

```text
CPU
 │
 ▼
GDTR
 │
 ▼
GDT
 ├── Null Descriptor
 ├── Kernel Code Segment
 ├── Kernel Data Segment
 ├── User Code Segment
 └── User Data Segment
```

### Deliverables

* GDT structure
* Segment descriptors
* GDT initialization
* Assembly reload
* Kernel code/data segmentation

---

# Phase 4 — Interrupt Descriptor Table

## Objective

Create the CPU interrupt and exception infrastructure.

### Architecture

```text
CPU
 │
 ├── Exception
 │
 └── Interrupt
        │
        ▼
       IDT
        │
        ▼
   ISR / Handler
        │
        ▼
      Kernel
```

### Target Exceptions

```text
Divide Error
Debug
Breakpoint
Invalid Opcode
Double Fault
General Protection Fault
Page Fault
...
```

### Deliverables

* IDT
* ISR stubs
* Exception handlers
* Interrupt frames
* Diagnostic output

---

# Phase 5 — PIC and Hardware IRQ

## Objective

Connect hardware devices to the kernel interrupt system.

### Architecture

```text
Hardware Device
      │
      ▼
   8259 PIC
      │
      ▼
     IRQ
      │
      ▼
  IDT Vector
      │
      ▼
 IRQ Handler
      │
      ▼
    Kernel
```

### Initial IRQ Mapping

```text
IRQ0 → Timer
IRQ1 → Keyboard
IRQ2 → Cascade
...
```

### Deliverables

* PIC initialization
* PIC remapping
* IRQ registration
* IRQ dispatch
* End-of-interrupt handling

---

# Phase 6 — Timer and Keyboard

## Timer Architecture

```text
PIT
 │
 ▼
IRQ0
 │
 ▼
Timer Handler
 │
 ▼
Tick Counter
 │
 ▼
Kernel Time
```

## Keyboard Architecture

```text
PS/2 Keyboard
      │
      ▼
     IRQ1
      │
      ▼
Keyboard Handler
      │
      ▼
Scancode Decoder
      │
      ▼
Keyboard Buffer
```

### Deliverables

* Timer ticks
* Kernel time accounting
* Keyboard interrupts
* Scancode processing
* Character input
* Input buffer

---

# Phase 7 — Physical Memory Manager

## Objective

Introduce explicit physical-memory management.

### Architecture

```text
Physical Memory
       │
       ▼
   Memory Map
       │
       ▼
Frame Allocator
       │
       ├── Allocate
       ├── Free
       └── Track
```

### Initial Design

```text
Physical Memory
       ↓
Bitmap
       ↓
Frame Allocation
```

### Future Designs

```text
Bitmap
  ↓
Buddy Allocator
  ↓
Slab Allocator
```

### Deliverables

* Physical memory map
* Frame allocator
* Bitmap
* `alloc_frame()`
* `free_frame()`
* Memory statistics

---

# Phase 8 — Process Management

## Objective

Introduce a real kernel process abstraction.

### Architecture

```text
                 Process Manager
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
         PID          PCB       Kernel Stack
          │
          ▼
      CPU Context
```

### Process States

```text
NEW
 │
 ▼
READY
 │
 ▼
RUNNING
 │
 ├──────► BLOCKED
 │
 ├──────► SLEEPING
 │
 └──────► TERMINATED
```

### Components

* PID allocation
* Process table
* Process states
* Process creation
* Process termination
* Kernel stacks
* CPU context
* Runtime accounting

### Current Direction

The kernel already contains process-management infrastructure and an assembly-backed context-switch path. The next objective is to turn that infrastructure into a fully preemptive scheduling subsystem.

---

# Phase 9 — Preemptive Scheduler

## Objective

Replace manual process switching with a real scheduler.

### Target Architecture

```text
                 Timer IRQ
                     │
                     ▼
              Scheduler Entry
                     │
                     ▼
              Save CPU Context
                     │
                     ▼
             Scheduler Algorithm
                     │
          ┌──────────┼──────────┐
          ▼          ▼          ▼
       Process A  Process B  Process C
          │          │          │
          └──────────┼──────────┘
                     ▼
               Next Process
                     │
                     ▼
               Context Switch
                     │
                     ▼
                  CPU Run
```

### First Scheduler

Implement:

```text
Round Robin
```

### Future Experiments

```text
Priority Scheduling
Multilevel Feedback Queue
Real-Time Scheduling
Fair Scheduling
```

### Deliverables

* Ready queue
* Scheduler abstraction
* Time slices
* Timer-driven preemption
* Context save/restore
* Process selection
* Scheduler statistics
* Fairness measurements

---

# Phase 10 — User Mode and System Calls

## Objective

Separate user-space applications from kernel-space execution.

### Architecture

```text
┌─────────────────────────────┐
│          USER SPACE         │
│                             │
│  Shell / Applications       │
└─────────────┬───────────────┘
              │
              │ System Call
              ▼
┌─────────────────────────────┐
│       SYSTEM CALL ABI       │
└─────────────┬───────────────┘
              │
              ▼
┌─────────────────────────────┐
│         KERNEL SPACE        │
│                             │
│ Process / Memory / Drivers  │
└─────────────────────────────┘
```

### Initial System Calls

```text
write()
read()
exit()
yield()
sleep()
getpid()
```

### Deliverables

* Ring 3 execution
* User stack
* TSS
* System-call entry
* System-call dispatcher
* Syscall table
* User-process execution

---

# Future Roadmap

The project continues beyond Phase 10.

---

## Phase 11 — Virtual Memory

```text
Virtual Address
      │
      ▼
Page Directory
      │
      ▼
Page Table
      │
      ▼
Physical Frame
```

Features:

* Paging
* Page directories
* Page tables
* Address spaces
* Page faults
* Kernel virtual memory
* User virtual memory

---

# Phase 12 — Kernel Heap

```text
kmalloc()
    │
    ▼
Kernel Heap
    │
    ├── Allocate
    ├── Free
    ├── Reallocate
    └── Statistics
```

Potential allocators:

* First Fit
* Best Fit
* Buddy
* Slab

---

# Phase 13 — Virtual File System

### Architecture

```text
Application
     │
     ▼
 System Call
     │
     ▼
     VFS
     │
 ┌───┼──────────┐
 ▼   ▼          ▼
FAT  EXT2      DevFS
```

The VFS will provide a common interface between user applications and filesystem implementations.

---

# Phase 14 — Filesystem

Initial target:

```text
RAM Filesystem
```

Future target:

```text
FAT32
```

Research target:

```text
Custom AyushOS Filesystem
```

---

# Phase 15 — Shell

### Architecture

```text
Keyboard
    │
    ▼
Input Buffer
    │
    ▼
Command Parser
    │
    ▼
Shell
    │
    ▼
System Calls
    │
    ▼
Kernel
```

### Example Commands

```text
AyushOS> help
AyushOS> clear
AyushOS> echo hello
AyushOS> ps
AyushOS> mem
AyushOS> uptime
AyushOS> uname
```

---

# Phase 16 — ELF Loader

## Objective

Load real executable programs.

```text
ELF Binary
    │
    ▼
ELF Parser
    │
    ▼
Program Headers
    │
    ▼
Virtual Memory
    │
    ▼
User Process
    │
    ▼
Ring 3
```

---

# Phase 17 — Networking

## Architecture

```text
Application
     │
     ▼
Socket API
     │
     ▼
TCP/IP Stack
     │
     ▼
Ethernet
     │
     ▼
NIC Driver
     │
     ▼
QEMU Network
```

Potential protocol stack:

```text
ARP
IPv4
ICMP
UDP
TCP
```

---

# Phase 18 — Security Research

AyushOS can become a controlled environment for experimenting with operating-system security.

Potential research areas:

* Kernel isolation
* User/kernel privilege separation
* Memory protection
* Capability systems
* Process sandboxing
* Secure syscall validation
* Stack protection
* Address-space randomization experiments
* Control-flow integrity
* Kernel attack-surface analysis
* Secure boot experiments

---

# Phase 19 — SMP and Multicore

### Architecture

```text
                  CPU 0
                    │
              ┌─────▼─────┐
              │ Scheduler │
              └─────┬─────┘
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
      CPU 0       CPU 1       CPU 2
        │           │           │
        └───────────┼───────────┘
                    ▼
              Shared Memory
```

Potential work:

* SMP boot
* APIC
* Per-CPU data
* Spinlocks
* Atomic operations
* Parallel scheduling
* Inter-processor interrupts

---

# Phase 20 — Operating-System Research Platform

The long-term objective is to turn AyushOS into a controlled environment for operating-system experimentation.

Potential research topics:

### Scheduler Research

Compare:

```text
Round Robin
Priority Scheduling
MLFQ
Real-Time Scheduling
```

Measure:

```text
Context-switch overhead
Scheduling latency
Throughput
Fairness
CPU utilization
```

### Memory Research

Compare:

```text
Bitmap Allocator
Buddy Allocator
Slab Allocator
```

Measure:

```text
Allocation latency
Fragmentation
Memory utilization
Free/allocated frame distribution
```

### Security Research

Experiment with:

```text
Process isolation
Memory isolation
Sandboxing
Capability systems
Kernel hardening
```

---

# Research Metrics

AyushOS should eventually measure its own behavior instead of only demonstrating functionality.

| Metric               | Purpose                     |
| -------------------- | --------------------------- |
| Context Switch Time  | Scheduler overhead          |
| Interrupt Latency    | Hardware responsiveness     |
| Scheduler Latency    | Process responsiveness      |
| CPU Utilization      | Scheduler efficiency        |
| Memory Fragmentation | Allocator quality           |
| Allocation Latency   | Memory performance          |
| Syscall Overhead     | User/kernel transition cost |
| IPC Latency          | Process communication       |
| Network Throughput   | Network performance         |

---

# Build Environment

AyushOS is primarily developed and tested on Linux.

For Arch Linux:

```bash
sudo pacman -S base-devel gcc nasm binutils grub xorriso qemu-desktop gdb
```

Verify the tools:

```bash
gcc --version
nasm --version
ld --version
grub-mkrescue --version
qemu-system-i386 --version
gdb --version
make --version
```

---

# Building AyushOS

Clone the repository:

```bash
git clone https://github.com/AyushjhaDevops/AyushOS.git
cd AyushOS
```

Build:

```bash
make
```

Build the ISO:

```bash
make iso
```

Run in QEMU:

```bash
make run
```

Clean build artifacts:

```bash
make clean
```

Debug:

```bash
make debug
```

---

# Development Workflow

```text
Create Feature
      │
      ▼
Design Architecture
      │
      ▼
Implement
      │
      ▼
Compile
      │
      ▼
Boot in QEMU
      │
      ▼
Test
      │
      ▼
Debug with GDB
      │
      ▼
Benchmark
      │
      ▼
Document
      │
      ▼
Commit
      │
      ▼
Push
```

---

# Commit Convention

Recommended commit format:

```text
feat: add physical frame allocator
feat: implement round robin scheduler
feat: add user mode transition
fix: correct IRQ acknowledgement
fix: repair process stack initialization
refactor: separate scheduler from process manager
test: add scheduler stress test
docs: document memory architecture
perf: reduce context switch overhead
```

---

# Testing Strategy

AyushOS will use multiple levels of testing.

## Build Tests

Verify:

```text
Source Compilation
Object Generation
Kernel Linking
ISO Generation
```

## Boot Tests

Verify:

```text
GRUB
Kernel Entry
GDT
IDT
PIC
Timer
Keyboard
```

## Kernel Tests

Verify:

```text
Process Creation
Process Termination
Memory Allocation
Context Switching
Scheduling
System Calls
```

## Stress Tests

Future stress tests:

```text
100+ process creation
Rapid context switching
Memory allocation/free loops
Scheduler fairness
Interrupt load
Syscall load
```

---

# Debugging

AyushOS is designed to be debugged with QEMU and GDB.

Typical debugging workflow:

```text
AyushOS
   │
   ▼
QEMU
   │
   ▼
GDB
   │
   ├── Breakpoints
   ├── Registers
   ├── Stack
   ├── Memory
   └── Instruction Trace
```

Useful debugging targets include:

```text
Kernel Entry
Interrupt Handlers
Timer IRQ
Keyboard IRQ
Process Creation
Context Switch
Scheduler
Page Fault
System Calls
```

---

# Project Status

The project should be considered an **experimental development kernel**.

| Phase | Component               | Status |
| ----: | ----------------------- | ------ |
|     0 | Development Environment | ✅      |
|     1 | Boot + Kernel Entry     | ✅      |
|     2 | Terminal + Logging      | ✅      |
|     3 | GDT                     | ✅      |
|     4 | IDT + Exceptions        | ✅      |
|     5 | PIC + IRQ               | ✅      |
|     6 | Timer + Keyboard        | ✅      |
|     7 | Physical Memory         | 🔄     |
|     8 | Process Management      | 🔄     |
|     9 | Preemptive Scheduler    | 🔄     |
|    10 | User Mode + Syscalls    | ⏳      |
|    11 | Virtual Memory          | ⏳      |
|    12 | Kernel Heap             | ⏳      |
|    13 | VFS                     | ⏳      |
|    14 | Filesystem              | ⏳      |
|    15 | Shell                   | ⏳      |
|    16 | ELF Loader              | ⏳      |
|    17 | Networking              | ⏳      |
|    18 | Security Research       | ⏳      |
|    19 | SMP                     | ⏳      |
|    20 | Research Platform       | ⏳      |

> **Status is based on implementation maturity, not simply the existence of source files.**

---

# What Makes AyushOS Different?

AyushOS is deliberately built from the lowest level upward.

Instead of starting with:

```text
Application
   ↓
Framework
   ↓
Library
```

AyushOS starts with:

```text
CPU
 ↓
Assembly
 ↓
Bootloader
 ↓
Kernel
 ↓
Interrupts
 ↓
Memory
 ↓
Processes
 ↓
Scheduler
 ↓
System Calls
 ↓
Applications
```

This makes the project useful for learning and experimenting with systems programming at the hardware/software boundary.

---

# Learning Outcomes

Working on AyushOS provides practical experience with:

* Operating-system architecture
* Computer architecture
* C programming
* x86 assembly
* CPU registers
* CPU privilege levels
* Interrupts
* Hardware IRQs
* Device drivers
* Process management
* CPU scheduling
* Memory management
* Virtual memory
* System calls
* Filesystems
* Networking
* Kernel debugging
* Concurrency
* Systems security
* Performance engineering

---

# Long-Term Architecture

The complete target architecture is:

```text
                         USER APPLICATIONS
                                │
                                ▼
                           USER SPACE
                                │
                                ▼
                       SYSTEM CALL ABI
                                │
                                ▼
┌──────────────────────────────────────────────────────┐
│                     KERNEL SPACE                     │
│                                                      │
│  ┌────────────┐  ┌────────────┐  ┌──────────────┐  │
│  │ Scheduler  │  │ Processes  │  │ System Calls │  │
│  └────────────┘  └────────────┘  └──────────────┘  │
│                                                      │
│  ┌────────────┐  ┌────────────┐  ┌──────────────┐  │
│  │ VM Manager │  │ Heap       │  │ VFS          │  │
│  └────────────┘  └────────────┘  └──────────────┘  │
│                                                      │
│  ┌────────────────────────────────────────────────┐  │
│  │                DEVICE DRIVERS                  │  │
│  │ Timer │ Keyboard │ Disk │ Console │ Network    │  │
│  └───────────────────────┬────────────────────────┘  │
└──────────────────────────┼───────────────────────────┘
                           │
                           ▼
                    x86 HARDWARE
                           │
             ┌─────────────┼─────────────┐
             ▼             ▼             ▼
            CPU          Memory        Devices
```

---

# Research Direction

The ultimate objective is not simply:

> "Make an OS that boots."

The objective is:

> **Build a controlled operating-system environment where kernel designs can be implemented, measured, compared, and researched.**

This opens the door to experiments in:

```text
Scheduling
    ↓
Memory Management
    ↓
Concurrency
    ↓
Security
    ↓
Filesystem Design
    ↓
Networking
    ↓
Multicore Systems
```

---

# Contribution Guidelines

Contributions are welcome once the core architecture becomes sufficiently stable.

Potential contribution areas:

* Device drivers
* Kernel subsystems
* Scheduler experiments
* Memory allocators
* Testing infrastructure
* Documentation
* Performance benchmarks
* Security research

Before contributing:

1. Understand the relevant subsystem.
2. Read its architecture documentation.
3. Keep changes isolated.
4. Add tests where practical.
5. Document architectural changes.
6. Use descriptive commit messages.

---

# Project Limitations

AyushOS is currently an experimental operating-system project.

It should **not** be used as:

* A production operating system
* A secure daily-driver OS
* A replacement for Linux/Windows
* A system for storing sensitive information
* A production networking platform

The project prioritizes learning, experimentation, and research.

---

# Roadmap Summary

```text
                 AYUSHOS
                    │
                    ▼
             ┌─────────────┐
             │ Bootloader  │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │   Kernel    │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │ Interrupts  │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │  Drivers    │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │   Memory    │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │  Processes  │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │ Scheduler   │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │  User Mode  │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │ System Call │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │   Virtual   │
             │   Memory    │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │ Filesystem  │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │ Networking  │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │  Security   │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │    SMP      │
             └──────┬──────┘
                    ▼
             ┌─────────────┐
             │  Research   │
             │  Platform   │
             └─────────────┘
```

---

# License

Add an explicit open-source license before accepting external contributions.

Recommended options include:

```text
MIT
BSD-2-Clause
BSD-3-Clause
Apache-2.0
GPL-2.0
```

Choose the license based on the project's intended distribution and contribution model.

---

# Author

## Ayush Kumar Jha

**Computer Science Engineering**

Interests:

```text
Operating Systems
Systems Programming
Cybersecurity
Artificial Intelligence
DevOps
Cloud Computing
```

---

# Project

**AyushOS**

> **From CPU instructions to operating-system research.**

---
