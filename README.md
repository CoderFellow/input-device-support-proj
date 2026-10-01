# Input Device Support Operating System - Project Documentation

## 1. Introduction



* **Project Name:** Input Device Support (`input-device-support-proj`)


* **Project Description:** This project is a bare-metal operating system prototype built to provide isolated emulation, processing, and visual rendering of keyboard and mouse input hardware.

* **Core Objectives:**
* Capture, decode, and process user keystrokes in real-time via hardware interrupt request lines.
* Implement full auxiliary PS/2 mouse support featuring a functional on-screen cursor.

---

## 2. System Overview

* **Problem Statement:** Modern high-level operating systems abstract hardware interactions completely, concealing the mechanics of boot sequences, interrupt vector management, and raw device polling. This project establishes a transparent, low-level environment to handle hardware handshakes directly.
* **Project Objectives:** Initialize bare-metal CPU structures, manage hardware interrupts, and cleanly coordinate communication between low-level input drivers and display subsystems.
* **Scope of the System:** Covers 16-bit to 32-bit CPU protected-mode switching, IDT registration for IRQ1 (keyboard) and IRQ12 (mouse), and framebuffer/VGA rendering.
* **System Limitations:** Restricted to x86 emulators (QEMU/Bochs) without multi-core Symmetric Multiprocessing (SMP) or advanced virtual memory paging.

---

## 3. Software Requirement Specifications (SRS)



* **Functional Requirements:**
* The system must read raw keyboard scancodes from port `0x60`, translate them into readable ASCII values, and process inputs reliably.


* The system must capture real-time 3-byte movement packets from the PS/2 mouse auxiliary port and render a functional cursor on screen.


* The kernel must seamlessly integrate asynchronous hardware events with graphic/text output to display live feedback.




* **Non-Functional Requirements:**
* **Performance:** Immediate response time to hardware interrupt requests with zero perceptible input lag.
* **Reliability:** Stable execution without triple faults or unhandled CPU exceptions during continuous input streaming.



---

## 4. Software Analysis and Design



* **System Architecture:** Written predominantly in C alongside low-level x86 Assembly.


* **Bootloader Design:** Handles CPU initialization, switches the processor from Real Mode to Protected Mode, and loads the binary kernel image into memory (`boot/boot.asm`, `boot/kernel_entry.asm`).


* **Kernel Design:** Manages core CPU operations, hardware handshakes, and sets up the Interrupt Descriptor Table (`kernel/idt.c`, `kernel/interrupt.asm`).


* **Graphics System Design:** Manages screen display configurations, video memory mapping, and pixel/cursor rendering (`kernel/graphics.c`).


* **User Input Handling:** Implements discrete device drivers for polling and handling hardware interrupts from keyboards (`kernel/keyboard.c`) and mice (`kernel/mouse.c`).



---

## 5. Development & Implementation



* **Bootloader Implementation:** Configures the 512-byte MBR sector and loads subsequent sectors using BIOS disk services.
* **Kernel Implementation:** Establishes stack pointers, maps the Global Descriptor Table (GDT), and transitions control to the main C entry point.
* **Graphics Implementation:** Initializes screen buffer addresses (`0xb8000` for text or linear framebuffers for graphics) to paint shapes and cursors.
* **User Input Handling:** Registers IRQ handlers via the Programmable Interrupt Controller (PIC) remapping sequence to capture scancodes and relative mouse movement deltas ($dX, dY$).

---

## 6. Testing & Validation



* **Unit Test:** Validates isolated components, such as scancode translation arrays or individual port reading helper functions (`inb`/`outb`).


* **Integration Test:** Ensures the IDT correctly routes keyboard (IRQ1) and mouse (IRQ12) hardware interrupts to their respective C driver handlers without dropping bits.


* **System Test:** Runs the complete compiled OS image (`os-image.bin`) inside an emulator environment (`qemu-system-x86_64`) to verify end-to-end functionality.


* **Performance Test:** Monitors CPU response times to asynchronous interrupt requests and reviews execution stability logs (`qemu_crash.log`).



---

## 7. Deployment



* **Emulator Deployment:** Executed primarily within QEMU using raw drive configurations (`qemu-system-x86_64 -drive format=raw,file=os-image.bin`).


* **Virtual Machine Setup:** Configured via modular Makefiles and linker scripts (`linker.ld`) to ensure seamless compilation across Linux and WSL developer environments.

---

## 8. Evaluation



* **System Performance Evaluation:** The kernel successfully processes asynchronous input interrupts concurrently. Keystrokes echo instantly to the screen, and mouse cursor tracking remains smooth within defined screen boundaries.
* **Limitations and Challenges:** Managing race conditions between simultaneous keyboard and mouse interrupts proved challenging, requiring careful circular buffer queuing and minimal overhead inside Interrupt Service Routines (ISRs).

---

## 9. Maintenance & Support



* **Conclusion:** The project successfully demonstrates bare-metal operating system development, proving that low-level hardware communication, interrupt management, and device drivers can be built entirely from scratch without standard library dependencies.
* **Future Enhancements:** Future iterations could expand support from legacy PS/2 devices to modern USB Human Interface Device (HID) protocols, add multi-tasking process scheduling, and implement dynamic GUI window management.

---

## 10. References



* *Operating Systems: Three Easy Pieces* (Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau).
* OSDev Wiki (Open Source Operating Systems Development Documentation - Port 0x60, PIC Remapping, and PS/2 Mouse Protocols).
* DWU MC415 Course Lecture Notes and Lab Manuals.
