#include <stdio.h>
#include <assert.h>
#include <stdint.h>

// Forward declaration of the parsing functions inside your drivers.
// (Make sure these functions are not declared 'static' in your .c files 
// so the linker can see them during host compilation!)
void mouse_parse_packet(uint8_t packet[3], int *dx, int *dy, int *left_click);
uint8_t keyboard_scancode_to_ascii(uint8_t scancode);

void test_mouse_parsing() {
    printf("[TEST] Running mouse packet parsing tests...\n");

    // Standard PS/2 3-byte packet mock:
    // Byte 0: Flags (e.g., bit 0 = Left Click active)
    // Byte 1: Delta X movement
    // Byte 2: Delta Y movement
    uint8_t mock_packet[3] = { 0x09, 0x0A, 0xFE }; 

    int dx = 0;
    int dy = 0;
    int left_click = 0;

    // Call your driver's logic function directly
    mouse_parse_packet(mock_packet, &dx, &dy, &left_click);

    // Assert that the packet was decoded properly according to your logic
    assert(left_click == 1);
    assert(dx == 10); // 0x0A = 10
    
    printf("[PASS] Mouse packet parsing test passed!\n");
}

void test_keyboard_translation() {
    printf("[TEST] Running keyboard scancode translation tests...\n");

    // Example: Standard PS/2 Set 1 scancode for 'A' is 0x1E
    uint8_t scancode_a = 0x1E;
    
    uint8_t ascii = keyboard_scancode_to_ascii(scancode_a);
    
    assert(ascii == 'a' || ascii == 'A');
    
    printf("[PASS] Keyboard translation test passed!\n");
}

int main() {
    printf("========================================\n");
    printf("   STARTING HOST-SIDE KERNEL TESTS      \n");
    printf("========================================\n");

    test_mouse_parsing();
    test_keyboard_translation();

    printf("========================================\n");
    printf("   ALL TESTS COMPLETED SUCCESSFULLY     \n");
    printf("========================================\n");
    return 0;
}
