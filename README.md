# AyushOS

> **A research-oriented operating system kernel built from scratch to explore low-level systems programming, x86 architecture, interrupts, process management, scheduling, memory management, and kernel design.**

[![Language](https://img.shields.io/badge/Language-C%20%2B%20NASM-blue)](https://github.com/AyushjhaDevops/AyushOS)
[![Architecture](https://img.shields.io/badge/Architecture-x86%2032--bit-orange)](https://github.com/AyushjhaDevops/AyushOS)
[![Bootloader](https://img.shields.io/badge/Bootloader-GRUB-green)](https://www.gnu.org/software/grub/)
[![Emulator](https://img.shields.io/badge/Emulator-QEMU-red)](https://www.qemu.org/)
[![Build](https://img.shields.io/badge/Build-GNU%20Make-yellow)](https://www.gnu.org/software/make/)
[![Status](https://img.shields.io/badge/Status-Active%20Development-blueviolet)](https://github.com/AyushjhaDevops/AyushOS)

---

## Table of Contents

* [About](#about)
* [Project Goals](#project-goals)
* [Current Status](#current-status)
* [Features](#features)
* [Architecture](#architecture)
* [Boot Flow](#boot-flow)
* [Technology Stack](#technology-stack)
* [Repository Structure](#repository-structure)
* [Development Phases](#development-phases)
* [Building AyushOS](#building-ayushos)
* [Running in QEMU](#running-in-qemu)
* [Debugging with GDB](#debugging-with-gdb)
* [Kernel Subsystems](#kernel-subsystems)
* [Development Workflow](#development-workflow)
* [Testing Strategy](#testing-strategy)
* [Roadmap](#roadmap)
* [Research Direction](#research-direction)
* [Learning Outcomes](#learning-outcomes)
* [Limitations](#limitations)
* [Contributing](#contributing)
* [License](#license)

---

# About

**AyushOS** is an experimental operating-system kernel developed from scratch for learning and researching how computers work at the hardware/software boundary.

The project starts at the bootloader and kernel-entry level and progressively introduces fundamental operating-system concepts:

* CPU execution
* x86 protected mode
* Kernel initialization
* Global Descriptor Table (GDT)
* Interrupt Descriptor Table (IDT)
* CPU exceptions
* Programmable Interrupt Controller (PIC)
* Hardware IRQs
* Timer interrupts
* Keyboard input
* Process management
* CPU context switching
* Physical memory management
* Scheduling
* User/kernel separation
* System calls
* Virtual memory
* Filesystems
* Networking
* Kernel security
* Multicore execution

The primary goal is **understanding how an operating system works internally**, rather than creating a production replacement for Linux or another mature operating system.

---

# Project Goals

AyushOS follows a bottom-up systems-engineering approach:

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

Each major subsystem is intended to eventually include:

* Implementation
* Architecture documentation
* Validation/testing
* Debugging support
* Performance measurements
* Design notes

---

# Current Status

AyushOS is currently an **experimental development kernel**.

The repository already contains the foundational x86 kernel infrastructure including:

* GRUB boot path
* x86 assembly bootstrap
* Kernel entry point
* VGA terminal output
* Kernel logging
* GDT
* IDT
* Interrupt/exception infrastructure
* PIC
* IRQ handling
* Timer infrastructure
* Keyboard driver
* Process-management infrastructure
* Assembly-backed context switching

The repository currently contains **14 commits** and is actively structured around incremental OS-development phases.

### Implementation Status

| Phase | Component                | Status |
| ----: | ------------------------ | :----: |
|     0 | Development Environment  |    ✅   |
|     1 | Boot + Kernel Entry      |    ✅   |
|     2 | Terminal + Logging       |    ✅   |
|     3 | GDT                      |    ✅   |
|     4 | IDT + Exceptions         |    ✅   |
|     5 | PIC + IRQ                |    ✅   |
|     6 | Timer + Keyboard         |    ✅   |
|     7 | Physical Memory          |   🔄   |
|     8 | Process Management       |   🔄   |
|     9 | Preemptive Scheduler     |   🔄   |
|    10 | User Mode + System Calls |    ⏳   |
|    11 | Virtual Memory           |    ⏳   |
|    12 | Kernel Heap              |    ⏳   |
|    13 | Virtual File System      |    ⏳   |
|    14 | Filesystem               |    ⏳   |
|    15 | Shell                    |    ⏳   |
|    16 | ELF Loader               |    ⏳   |
|    17 | Networking               |    ⏳   |
|    18 | Security Research        |    ⏳   |
|    19 | SMP / Multicore          |    ⏳   |
|    20 | OS Research Platform     |    ⏳   |

> **Legend:**
> ✅ Implemented foundation
> 🔄 Currently being developed
> ⏳ Planned / future work

The status above reflects implementation maturity rather than simply whether a source file exists.

---

# Features

## Currently Implemented / In Development

### Boot Process

AyushOS uses GRUB to load the kernel and transfer execution to the kernel entry point.

```text
BIOS / Firmware
      ↓
     GRUB
      ↓
  boot.asm
      ↓
Kernel Entry
      ↓
kernel_main()
```

---

## Kernel Terminal

The kernel provides low-level terminal functionality using VGA text memory.

Current functionality includes:

* Character output
* String output
* Integer formatting
* Kernel logging
* Diagnostic messages
* Debug output

Example:

```text
[ OK ] Terminal initialized
[ OK ] GDT initialized
[ OK ] IDT initialized
[ OK ] PIC initialized
[INFO] Kernel boot complete
```

---

# GDT

The Global Descriptor Table establishes the segmentation model required by the x86 protected-mode environment.

The architecture includes descriptors for:

```text
GDT
├── Null Descriptor
├── Kernel Code Segment
├── Kernel Data Segment
├── User Code Segment
└── User Data Segment
```

---

# IDT and Interrupts

The Interrupt Descriptor Table provides the foundation for CPU exceptions and hardware interrupts.

```text
CPU
 │
 ├── Exception
 │
 └── Interrupt
       ↓
      IDT
       ↓
   ISR / Handler
       ↓
     Kernel
```

The design includes infrastructure for handling common CPU exceptions such as:

* Divide Error
* Debug
* Breakpoint
* Invalid Opcode
* Double Fault
* General Protection Fault
* Page Fault

---

# PIC and Hardware IRQs

AyushOS uses the traditional x86 Programmable Interrupt Controller infrastructure.

```text
Hardware Device
      ↓
    8259 PIC
      ↓
     IRQ
      ↓
  IDT Vector
      ↓
 IRQ Handler
      ↓
   Kernel
```

Initial hardware mapping includes:

```text
IRQ0 → Timer
IRQ1 → Keyboard
IRQ2 → Cascade
```

---

# Timer

The timer subsystem receives periodic interrupts from the PIT.

```text
PIT
 ↓
IRQ0
 ↓
Timer Handler
 ↓
Tick Counter
 ↓
Kernel Time
```

Timer infrastructure is important for future preemptive scheduling.

---

# Keyboard Driver

AyushOS includes a PS/2 keyboard input path.

```text
PS/2 Keyboard
      ↓
     IRQ1
      ↓
Keyboard Handler
      ↓
Scancode Decoder
      ↓
Keyboard Buffer
```

The current architecture supports:

* Keyboard interrupts
* Scancode processing
* Character input
* Input buffering

---

# Process Management

The kernel contains an early process-management subsystem.

A process is represented using concepts such as:

* PID
* Process state
* Process table
* Kernel stack
* CPU context
* Runtime information

Process states are designed around:

```text
NEW
 ↓
READY
 ↓
RUNNING
 ├── BLOCKED
 ├── SLEEPING
 └── TERMINATED
```

The repository also contains an assembly-backed context-switching path. The next major step is turning this foundation into a complete preemptive scheduler.

---

# Architecture

## Current Kernel Architecture

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
              ┌─────────────────┼─────────────────┐
              │                 │                 │
              ▼                 ▼                 ▼
          ┌────────┐        ┌────────┐        ┌────────┐
          │  GDT   │        │  IDT   │        │  PIC   │
          └────────┘        └────┬───┘        └────┬───┘
                                  │                 │
                                  └────────┬────────┘
                                           ▼
                                  ┌────────────────┐
                                  │ IRQ Subsystem  │
                                  └───────┬────────┘
                                          │
                           ┌──────────────┼──────────────┐
                           │              │              │
                           ▼              ▼              ▼
                       ┌────────┐    ┌──────────┐   ┌──────────┐
                       │ Timer  │    │ Keyboard │   │ Future   │
                       │ IRQ0   │    │   IRQ1   │   │ Devices  │
                       └────┬───┘    └─────┬────┘   └──────────┘
                            │               │
                            └───────┬───────┘
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
                        ┌────────────┼────────────┐
                        ▼            ▼            ▼
                    Process A    Process B      Idle
```

---

# Long-Term Architecture

The intended complete architecture is:

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
┌────────────────────────────────────────────────────┐
│                    KERNEL SPACE                    │
│                                                    │
│  Process Manager      Scheduler                   │
│         │                 │                        │
│         └────────┬────────┘                        │
│                  ▼                                 │
│            Memory Manager                          │
│                  │                                 │
│         ┌────────┴────────┐                        │
│         ▼                 ▼                        │
│   Virtual Memory      Kernel Heap                  │
│                                                    │
│  VFS → Filesystems → Storage                       │
│                                                    │
│  Network Stack → NIC Drivers                       │
│                                                    │
│  Security / Isolation / IPC                        │
│                                                    │
│  Hardware Drivers                                  │
└───────────────────────┬────────────────────────────┘
                        │
                        ▼
                  x86 HARDWARE
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
| Host Development OS | Arch Linux                 |
| Version Control     | Git / GitHub               |

The repository currently targets a 32-bit x86 environment and uses C, NASM, GRUB, GCC, GNU Make, QEMU and GDB as its primary development stack.

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

This structure cleanly separates architecture-specific assembly, kernel subsystems, headers, drivers, libraries, and build/linker configuration.

---

# Development Phases

## Phase 0 — Development Environment

* GCC
* NASM
* GNU Binutils
* GRUB
* Xorriso
* QEMU
* GDB
* GNU Make

---

## Phase 1 — Boot + Kernel Entry

* GRUB boot
* Assembly bootstrap
* Kernel entry
* Linker script
* Bootable ISO

---

## Phase 2 — Terminal + Logging

* VGA text mode
* Character output
* String output
* Integer formatting
* Kernel logging
* Debug messages

---

## Phase 3 — GDT

* GDT structure
* Segment descriptors
* Kernel code/data segments
* User code/data segment preparation
* Assembly reload

---

## Phase 4 — IDT + Exceptions

* IDT
* ISR stubs
* Exception handlers
* Interrupt frames
* Diagnostic output

---

## Phase 5 — PIC + IRQ

* PIC initialization
* PIC remapping
* IRQ registration
* IRQ dispatch
* End-of-interrupt handling

---

## Phase 6 — Timer + Keyboard

* PIT timer
* Timer ticks
* Keyboard IRQ
* Scancode decoding
* Character input
* Input buffer

---

## Phase 7 — Physical Memory

Planned/under development:

* Physical memory map
* Frame allocator
* Bitmap allocator
* Frame allocation/freeing
* Memory statistics

---

## Phase 8 — Process Management

Under development:

* PID allocation
* Process table
* Process states
* Process creation
* Process termination
* Kernel stacks
* CPU context
* Context switching

---

## Phase 9 — Preemptive Scheduler

Target:

```text
Timer IRQ
   ↓
Scheduler
   ↓
Save Context
   ↓
Select Process
   ↓
Context Switch
   ↓
Run Process
```

Initial scheduler:

**Round Robin**

Future experiments:

* Priority scheduling
* Multilevel Feedback Queue
* Real-time scheduling
* Fair scheduling

---

## Phase 10 — User Mode + System Calls

Target system calls:

```text
write()
read()
exit()
yield()
sleep()
getpid()
```

Main objectives:

* Ring 3 execution
* User stack
* TSS
* System-call entry
* System-call dispatcher
* Syscall table
* User processes

---

# Future Roadmap

## Phase 11 — Virtual Memory

* Paging
* Page directories
* Page tables
* Address spaces
* Page faults
* Kernel virtual memory
* User virtual memory

---

## Phase 12 — Kernel Heap

Introduce dynamic kernel allocation:

```text
kmalloc()
kfree()
krealloc()
```

Possible allocator implementations:

* First Fit
* Best Fit
* Buddy
* Slab

---

## Phase 13 — Virtual File System

Create a generic filesystem abstraction:

```text
Application
     ↓
System Call
     ↓
    VFS
     ↓
 ┌───┼──────────┐
 ▼   ▼          ▼
FAT EXT2      DevFS
```

---

## Phase 14 — Filesystem

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

## Phase 15 — Shell

Target architecture:

```text
Keyboard
   ↓
Input Buffer
   ↓
Command Parser
   ↓
Shell
   ↓
System Calls
   ↓
Kernel
```

Example future commands:

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

## Phase 16 — ELF Loader

Load actual executable programs:

```text
ELF Binary
    ↓
ELF Parser
    ↓
Program Headers
    ↓
Virtual Memory
    ↓
User Process
    ↓
Ring 3
```

---

## Phase 17 — Networking

Potential stack:

```text
Application
     ↓
 Socket API
     ↓
  TCP/IP
     ↓
 Ethernet
     ↓
 NIC Driver
     ↓
 QEMU Network
```

Potential protocols:

* ARP
* IPv4
* ICMP
* UDP
* TCP

---

## Phase 18 — Security Research

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

## Phase 19 — SMP / Multicore

Future work:

* SMP boot
* APIC
* Per-CPU data
* Spinlocks
* Atomic operations
* Parallel scheduling
* Inter-processor interrupts

---

## Phase 20 — OS Research Platform

The long-term objective is to use AyushOS as a controlled environment for experimenting with operating-system design.

Potential research areas include:

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
Frame distribution
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

# Building AyushOS

## Requirements

A suitable development environment should provide:

```text
GCC
NASM
GNU Binutils
GRUB
Xorriso
QEMU
GDB
GNU Make
```

The project is currently documented around an Arch Linux development environment.

## Clone

```bash
git clone https://github.com/AyushjhaDevops/AyushOS.git
cd AyushOS
```

## Build

```bash
make
```

The Makefile is responsible for compiling the kernel sources, assembling low-level x86 code, linking the kernel, and preparing the bootable image.

---

# Running in QEMU

After building the project, boot the generated image using QEMU.

Typical workflow:

```bash
make
```

Then launch the generated kernel/ISO with your QEMU configuration.

For example:

```bash
qemu-system-i386 -cdrom build/ayushos.iso
```

> The exact output image path depends on the current Makefile configuration. Verify the generated artifact before running the command.

---

# Debugging with GDB

AyushOS is designed to be debugged at the kernel level using QEMU and GDB.

Typical workflow:

```text
AyushOS
   ↓
 QEMU
   ↓
 GDB
   ├── Breakpoints
   ├── Registers
   ├── Stack
   ├── Memory
   └── Instruction Trace
```

Useful debugging targets include:

* Kernel entry
* GDT initialization
* IDT initialization
* Interrupt handlers
* Timer IRQ
* Keyboard IRQ
* Process creation
* Context switching
* Scheduler
* Page faults
* System calls

---

# Kernel Subsystems

The project is organized around the following major subsystem boundaries:

| Subsystem        | Purpose                      |
| ---------------- | ---------------------------- |
| Boot             | Starts the kernel            |
| GDT              | x86 segmentation             |
| IDT              | Interrupt/exception dispatch |
| PIC              | Hardware interrupt routing   |
| IRQ              | Hardware interrupt handling  |
| Timer            | Periodic kernel interrupts   |
| Keyboard         | Hardware input               |
| Terminal         | Kernel display               |
| Logging          | Kernel diagnostics           |
| Process Manager  | Process lifecycle            |
| Context Switcher | CPU context switching        |
| Memory Manager   | Physical memory management   |
| Scheduler        | CPU time allocation          |
| Syscall Layer    | User/kernel interface        |
| VMM              | Virtual memory               |
| Heap             | Dynamic kernel allocation    |
| VFS              | Filesystem abstraction       |
| Network Stack    | Network communication        |
| Security         | Kernel isolation/hardening   |
| SMP              | Multicore support            |

---

# Development Workflow

AyushOS follows a low-level iterative development cycle:

```text
Create Feature
      ↓
Design Architecture
      ↓
Implement
      ↓
Compile
      ↓
Boot in QEMU
      ↓
Test
      ↓
Debug with GDB
      ↓
Benchmark
      ↓
Document
      ↓
Commit
      ↓
Push
```

---

# Testing Strategy

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

Future validation should cover:

```text
Process Creation
Process Termination
Memory Allocation
Context Switching
Scheduling
System Calls
```

## Stress Tests

Planned stress tests include:

```text
100+ process creation
Rapid context switching
Memory allocation/free loops
Scheduler fairness
Interrupt load
Syscall load
```

---

# Performance Metrics

The project is intended to evolve from a functional educational kernel into a measurable research platform.

Potential metrics include:

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

# What Makes AyushOS Different?

AyushOS intentionally starts from the lowest level rather than beginning with an application framework.

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

The purpose is to understand the layers that normally remain hidden behind mature operating systems.

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

# Limitations

AyushOS is an **experimental educational kernel**, not a production operating system.

Current limitations include:

* 32-bit x86 target
* Limited hardware support
* Early-stage process management
* Incomplete scheduler
* No complete user-space environment yet
* Virtual memory still planned
* Filesystem support still planned
* Networking still planned
* SMP/multicore support still planned
* Limited automated testing infrastructure

These limitations are intentional milestones in the project's development rather than claims of production readiness.

---

# Project Philosophy

> **Build from the hardware upward. Understand every layer. Measure what you build.**

AyushOS is not primarily about making another operating system.

It is about understanding:

```text
What happens after the computer turns on?

How does the CPU enter the kernel?

How does the kernel handle interrupts?

How does memory get allocated?

How are processes created?

How does the CPU switch between processes?

How does user code communicate with the kernel?

How does a filesystem reach a storage device?

How does a network packet reach an application?

How can all of these components be isolated and secured?
```

The project attempts to answer these questions by implementing the underlying mechanisms directly.

---

# Contributing

Contributions, experiments, documentation improvements, bug fixes, and architectural discussions are welcome.

Before contributing:

1. Understand the affected subsystem.
2. Document the intended design.
3. Implement the smallest testable change.
4. Build the kernel.
5. Boot it in QEMU.
6. Test the affected subsystem.
7. Debug with GDB when necessary.
8. Document important architectural decisions.

Suggested commit style:

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

# Project Direction

The immediate development direction is:

```text
Physical Memory
      ↓
Process Management
      ↓
Preemptive Scheduler
      ↓
User Mode
      ↓
System Calls
      ↓
Virtual Memory
      ↓
Kernel Heap
      ↓
Filesystem
      ↓
Shell
      ↓
ELF Programs
      ↓
Networking
      ↓
Security
      ↓
SMP
      ↓
OS Research Platform
```

---

# License

See the repository for the current license information.

---

# Author

**Ayush Jha**

GitHub:

https://github.com/AyushjhaDevops

Project:

https://github.com/AyushjhaDevops/AyushOS

---

# Final Note

AyushOS is a journey from:

```text
Bare Metal
    ↓
Boot
    ↓
Kernel
    ↓
Interrupts
    ↓
Hardware
    ↓
Memory
    ↓
Processes
    ↓
Scheduling
    ↓
User Space
    ↓
Applications
```

The objective is not simply to produce a bootable kernel.

The objective is to understand **why an operating system works the way it does**.

**Build it. Boot it. Break it. Debug it. Measure it. Understand it.**
