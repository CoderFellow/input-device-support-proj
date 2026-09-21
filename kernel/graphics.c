#include "kernel.h"

// Basic VESA / Linear Framebuffer stub for MC415 Major Project Graphics Requirements
// Default physical address often used for VBE LFB or VGA memory mapping
#define VGA_GRAPHICS_ADDRESS 0xFA000000

void graphics_init(void) {
    // Placeholder for graphics mode initialization
    // In a full implementation, this sets up VESA BIOS extensions or mode 0x13
}

void draw_pixel(int x, int y, uint8_t color) {
    // Basic pixel plotter mapping (assuming a standard 800x600 8-bit/32-bit buffer)
    volatile uint32_t* screen = (volatile uint32_t*)0xA0000; 
    if (x >= 0 && x < 800 && y >= 0 && y < 600) {
        screen[y * 800 + x] = color;
    }
}