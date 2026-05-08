# IRQWATCH Integration Report

## Overview
This repo adds a kernel component that runs from boot and logs selected interrupts. It tracks timer, UART (keyboard/console), and virtio disk interrupts and prints a periodic summary.

## Files Changed/Added
- New: kernel/irqwatch.c (interrupt counters and periodic logging)
- Updated: kernel/main.c (boot-time init)
- Updated: kernel/trap.c (interrupt hooks)
- Updated: kernel/defs.h (prototypes)
- Updated: Makefile (adds irqwatch.o)
- Updated: README (project-level documentation)

## Invocation Flow
- Boot: kernel/main.c calls irqwatch_init() on hart 0 after trapinit().
- Interrupt path (kernel/trap.c -> devintr()):
  - Timer: clockintr() then irqwatch_trap_timer()
  - UART: uartintr() then irqwatch_trap_uart()
  - Disk: virtio_disk_intr() then irqwatch_trap_disk()

## Build and Run
- Build + run in QEMU: make qemu
- Clean build (if needed): make clean then make qemu

## Expected Output
- Periodic summary line (every 100 timer interrupts by default):
  irqwatch: timer=... uart=... disk=...

## Tuning the Logging
- Change IRQWATCH_PRINT_EVERY in kernel/irqwatch.c to adjust frequency.
- Add or remove interrupt types by editing hooks in kernel/trap.c and counters in kernel/irqwatch.c.

## Testing Checklist
- Boot test: make qemu and verify irqwatch summary lines appear.
- UART test: type in console; UART count should increase.
- Disk test: run ls or cat; disk count should increase.

## Safe Development Guidelines
- Keep interrupt-path work minimal and non-blocking.
- Avoid heavy printf usage inside devintr(); use counters and periodic summaries.
- Protect shared counters with locks (already done in irqwatch).
- If you add new interrupts, update both trap hooks and irqwatch counters.
- For more complex work, move it out of the interrupt path into periodic kernel logic.
# IRQWATCH Integration Report

## Summary
This repo integrates a small kernel component ("irqwatch") that runs from boot and tracks selected interrupts (timer, UART/keyboard/console, virtio disk). It logs a periodic summary to the console to keep noise low.

## Files Changed/Added
- New: kernel/irqwatch.c
- Updated: kernel/main.c (boot-time init)
- Updated: kernel/trap.c (interrupt hooks)
- Updated: kernel/defs.h (prototypes)
- Updated: Makefile (builds irqwatch.o)
- Updated: README (documents irqwatch)

## Invocation Flow
- Boot: main() calls irqwatch_init() after trapinit().
- Interrupt hooks:
  - Timer: devintr() calls irqwatch_trap_timer() after clockintr().
  - UART: devintr() calls irqwatch_trap_uart() after uartintr().
  - Disk: devintr() calls irqwatch_trap_disk() after virtio_disk_intr().

## Build and Run
- Build and run in QEMU:
  - make qemu
- Clean build (if needed):
  - make clean
  - make qemu

## What You Will See
- Every 100 timer interrupts (default), a summary line prints:
  - irqwatch: timer=... uart=... disk=...

## Tuning
- Adjust log cadence by changing IRQWATCH_PRINT_EVERY in kernel/irqwatch.c.
- To track different interrupt types, update the hook points in kernel/trap.c and
  add matching counters in kernel/irqwatch.c.

## Testing Suggestions
- Boot test: run make qemu and verify periodic irqwatch lines appear.
- Keyboard test: type in the console; UART count should increase.
- Disk test: run ls/cat; disk count should increase.

## Safe Development Guidelines
- Keep interrupt-path work short and non-blocking.
- Prefer counters with periodic summaries over per-interrupt prints.
- Protect shared counters with locks.
- If adding new interrupt types, add minimal hooks in trap.c and update irqwatch.c.
