# input device support

## Keyboard Driver (IRQ1)

### Reliable interrupt-driven keyboard input, scancode translation, and circular buffering.

> PIC & IDT Initialization:** Remap the Programmable Interrupt Controller (PIC) chips (Master and Slave) and write an assembly stub to register the Interrupt Service Routine (ISR) for IRQ1 at interrupt vector `0x21`.
> Port Communication (`inb`):** Implement the low-level port reading helper function to poll or capture raw Set 1 scancodes coming from IO port `0x60`.
> Scancode Translation & Circular Queue:** Build a scancode-to-ASCII lookup array (handling key presses vs. releases via the `0x80` break bit) and push processed characters into a thread-safe circular buffer queue.
> Milestone Check:** Type keys inside QEMU and verify they echo back correctly through your kernel's text or graphics output buffer.

---

## PS/2 Mouse Driver (IRQ12)

### auxiliary mouse device, catch 3-byte movement packets, and manage screen boundaries.

> Controller Enable:** Send command byte `0xA8` to the PS/2 controller status port (`0x64`) to enable the auxiliary mouse port, and enable IRQ12 (vector `0x2C`). Set up the mouse streaming mode (`0xF4`).


> **Packet Stream Parsing:** Catch incoming interrupts, maintaining a 3-byte packet index counter:
> Byte 1: Status flags (Y overflow, X overflow, Sign bits, Left/Right button states).
> Byte 2: Relative movement delta $dX$.
> Byte 3: Relative movement delta $dY$.


* **Boundary Clamping:** Convert relative deltas to absolute screen coordinates $(X, Y)$ and clamp them strictly within your current graphics mode resolution limits (e.g., $800 \times 600$ or $1024 \times 768$).

---

#### **Block 3: Evening — Framebuffer Cursor & Report Documentation**

*Objective: Render a stable visual cursor and capture your testing telemetry for the project report.*

* **Double-Buffering / Pixel Saving:** Render a custom cursor pointer shape onto the VBE/linear framebuffer. Implement a background save-and-restore routine for underlying pixels to prevent visual trailing and ghosting artifacts.
* **System Testing & Validation:** Boot the fully integrated kernel image inside `qemu-system-x86_64`. Perform simultaneous typing and mouse dragging to ensure no deadlock or interrupt starvation occurs.


* **Report Logging:** Take screenshots of your QEMU window running the interactive input handlers. Immediately draft the text for **Section 5.4 (User Input Handling Implementation)** and **Section 6 (Testing and Validation)** for your major project report.



---

### ## Major Project Artifact Checklist

As you finish each block today, make sure you save these items to drop straight into your final submission:

* [ ] **Source Code Files:** Cleanly commented C and assembly code for `keyboard.c`, `mouse.c`, `idt.asm`, and `isr.c`.
* [ ] **Test Evidence:** QEMU terminal/graphic execution logs or screenshots showing keyboard echo and mouse cursor tracking.
* [ ] **Drafting Content:** Bullet points detailing your interrupt vector numbers, port maps, and byte-parsing logic for Section 5.4 of the report.



---

Are you ready to tackle the Morning Block, and do you want to start with the assembly stub for the IDT setup or the C port-reading functions first?