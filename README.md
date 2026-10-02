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



* **Bootloader Implementation**: 
  * Structured as a 512-byte Master Boot Record (MBR) sector (ending with the `0xAA55` boot signature) starting at origin `0x7C00`.
    [org 0x7c00]
    bits 16

    _start:
        cli
        xor ax, ax
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov sp, 0x7C00

  * Utilizes BIOS interrupt `0x13` (CHS disk reading) to load multiple sectors from disk into memory at `0x9000`.
    .read_loop:
    mov ah, 0x02            ; BIOS read sector function
    mov dl, [BOOT_DRIVE]
    int 0x13
    jc .disk_error          ; If carry flag set, read failed

    ; Enable A20 Gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Switch to Protected Mode
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:init_pm    

  * Enables the A20 line, loads a Global Descriptor Table (GDT), sets up protected mode via `cr0`, and jumps into the 32-bit execution environment.

* **Kernel Implementation**: 
  * Governed by a custom linker script (`linker.ld`) starting at physical address `0x9000`.
    . = 0x9000;
    .text : {
        boot/kernel_entry.o(.text)
        *(.text)
    }
  
  * `kernel_main` (`kernel/kernel.c`) initializes serial output debugging (`0x3F8`), sets up the Interrupt Descriptor Table (`kernel/idt.c`), reprograms and remaps the PIC vector offsets, and enables global hardware interrupts via the `sti` instruction.
    static inline void serial_putchar(char c) {
    __asm__ volatile ("outb %0, %1" : : "a"(c), "Nd"((uint16_t)0x3F8));
    }

* **Graphics Implementation**: 
  * Manages text rendering by writing characters and attribute bytes directly to the physical VGA text-mode memory buffer (`0xB8000`).

  * Includes foundational stubs for VESA Linear Framebuffer pixel plotting (`kernel/graphics.c`).

* **User Input Handling**: 
  * **Keyboard Driver (`kernel/keyboard.c`)**: Maps interrupt vector `33` (`0x21`) to an assembly stub (`keyboard_handler_stub`), reads raw scancodes from port `0x60`, translates inputs via an ASCII conversion map, and updates sequential text coordinates on the VGA screen.
    ```c
    void keyboard_install(void) {
        // Map IRQ1 (keyboard) to offset 0x21 or your IDT vector entry
        idt_set_gate(33, (uint32_t)keyboard_handler_stub, 0x08, 0x8E);

        // Enable keyboard interrupt on the PIC (Clear mask for IRQ 1)
        uint8_t mask = inb(0x21);
        outb(0x21, mask & ~(1 << 1));
    }
    ```
  * **Mouse Driver (`kernel/mouse.c`)**: Enables the auxiliary PS/2 mouse channel through port `0x64`, initiates packet data streaming via command `0xF4`, parses 3-byte movement delta packets with sign extension, clamps coordinate bounds (0 to 79 X-axis, 0 to 24 Y-axis), and renders an active inverse-color block cursor.
    ```c
    void mouse_handler_main(void) {
        uint8_t status = inb(0x64);
        if (status & 0x01) { // Data available
            if (status & 0x20) { // Mouse data bit set
                mouse_packet[mouse_cycle++] = inb(0x60);
                if (mouse_cycle == 3) {
                    mouse_cycle = 0;
                    
                    int rel_x = (int)mouse_packet[1];
                    int rel_y = (int)mouse_packet[2];
                    
                    if (mouse_packet[0] & 0x10) rel_x |= 0xFFFFFF00; // Sign extend X
                    if (mouse_packet[0] & 0x20) rel_y |= 0xFFFFFF00; // Sign extend Y
                    
                    mouse_x += rel_x / 2;
                    mouse_y -= rel_y / 2; // Invert axis logic
                    
                    if (mouse_x < 0) mouse_x = 0;
                    if (mouse_x > 79) mouse_x = 79;
                    if (mouse_y < 0) mouse_y = 0;
                    if (mouse_y > 24) mouse_y = 24;
                    
                    draw_mouse_cursor(mouse_x, mouse_y);
                }
            }
        }
        outb(0xA0, 0x20); // Send EOI to Slave PIC
        outb(0x20, 0x20); // Send EOI to Master PIC
    }
    ```

---

## 6. Testing & Validation



* **Unit Test**: 
  * Validating isolated driver components, such as verifying that the scancode-to-ASCII translation lookup array correctly maps key presses.
  * Testing individual algorithmic logic like the mouse packet sign-extension and axis-clamping functions (`mouse_x` and `mouse_y` boundary checks)[cite: 2].
* **Integration Test**: 
  * Ensuring the Interrupt Descriptor Table (IDT) and Programmable Interrupt Controller (PIC) correctly route hardware interrupts (IRQ1 for the keyboard and IRQ12 for the mouse) to their respective assembly stubs and C handlers (`keyboard_handler_main` and `mouse_handler_main`)[cite: 2].
  * Verifying that proper End-of-Interrupt (EOI) commands (`outb(0x20, 0x20)`) are dispatched to prevent interrupt lockups or starvation[cite: 2].
* **System Test**: 
  * Deploying and executing the compiled OS floppy image (`os-image.bin`) within a QEMU x86 emulator environment.
  * Performing live end-to-end verification by typing characters to update the VGA text buffer (`0xB8000`) and moving the physical mouse to render the inverse-color block cursor smoothly within screen boundaries ($0$ to $79$ X-axis, $0$ to $24$ Y-axis)[cite: 2].
* **Performance Test**: 
  * Monitoring CPU response latency to high-frequency asynchronous hardware interrupts from input peripherals.
  * Tracking stability and inspecting serial debugging output (`0x3F8`) to confirm the kernel handles sustained inputs without exceptions or crashes.


---

## 7. Deployment

* **Emulator Deployment (QEMU/Bochs)**: 
  * The primary deployment pipeline runs the generated raw flat binary image (`os-image.bin`) inside an x86 hardware emulator.
  * Executed via QEMU using the standard system emulator target:
    ```bash
    qemu-system-i386 -fda os-image.bin
    ```
  * Configures emulated hardware components including the 8259 Programmable Interrupt Controller, PS/2 controller ports (`0x60`, `0x64`), and direct VGA text-mode memory output (`0xB8000`).
* **Raspberry Pi Deployment**: 
  * *Note on Architecture*: Because this custom kernel targets the 32-bit x86 (IA-32) architecture, booting natively on a Raspberry Pi (ARM-based BCM SoC) requires either an x86 emulation layer or cross-compilation to ARM bare-metal specifications (such as configuring the ARM vector table and PL011 UART/GPIO controllers). For this project's scope, physical deployment remains focused on x86-compatible environments.
* **Virtual Machine Setup**: 
  * Configured for deployment inside hypervisors such as Oracle VirtualBox or VMware Workstation by attaching `os-image.bin` as a legacy 1.44MB floppy disk image or raw bootable storage medium.
  * Ensures proper BIOS legacy boot sequencing (`0x7C00` MBR signature check) before handing control over to the protected-mode kernel at physical address `0x9000`.

---

## 8. Evaluation

* **System Performance Evaluation**:
  * **Interrupt Latency & Responsiveness**: The kernel achieves near-instantaneous CPU response times for hardware inputs. By routing interrupts directly through the Interrupt Descriptor Table (IDT) via optimized assembly stubs (`keyboard_handler_stub` and `mouse_handler_stub`)[cite: 2], the delay between a physical keystroke or mouse movement and its visual execution is minimized.
  * **Resource Efficiency**: Compiled freestanding (`-ffreestanding`, `-O2`) without standard library overhead, the kernel maintains an extremely lightweight footprint. The entire bootable OS image (`os-image.bin`) fits cleanly into a standard 1.44MB storage format and loads into memory instantaneously.
  * **Visual Feedback & Rendering**: The mouse driver effectively processes 3-byte movement packets, handles sign extension, clamps coordinates securely within screen boundaries ($0$ to $79$ X-axis, $0$ to $24$ Y-axis), and renders an active inverse-color block cursor (`0x70`) onto the VGA text buffer (`0xB8000`) without visual artifacting or stutter[cite: 2].

* **Limitations and Challenges**:
  * **Asynchronous Interrupt Synchronization**: Managing concurrent, unpredictable hardware events from multiple peripherals (IRQ1 for keyboard and IRQ12 for the mouse) required meticulous PIC vector remapping and precise End-of-Interrupt (EOI) signaling (`outb(0x20, 0x20)` and `outb(0xA0, 0x20)`)[cite: 2] to prevent interrupt starvation or deadlocks.
  * **Low-Level Development Constraints**: Operating entirely without standard library support meant all utilities—such as memory management, input parsing, and debugging hooks—had to be implemented from scratch using raw port I/O (`0x60`, `0x64`, `0x3F8`) and direct memory mapping.
  * **Toolchain and Environment Compatibility**: Overcoming cross-platform compilation quirks, linker script layout configurations (`linker.ld`), and binary format conversions during the build pipeline required strict attention to low-level binary specifications.

---

## 9. Maintenance & Support



* **Conclusion:** The project successfully demonstrates bare-metal operating system development, proving that low-level hardware communication, interrupt management, and device drivers can be built entirely from scratch without standard library dependencies.
* **Future Enhancements:** Future iterations could expand support from legacy PS/2 devices to modern USB Human Interface Device (HID) protocols, add multi-tasking process scheduling, and implement dynamic GUI window management.

---

## 10. References



* *Operating Systems: Three Easy Pieces* (Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau).
* OSDev Wiki (Open Source Operating Systems Development Documentation - Port 0x60, PIC Remapping, and PS/2 Mouse Protocols).
* DWU MC415 Course Lecture Notes and Lab Manuals.
