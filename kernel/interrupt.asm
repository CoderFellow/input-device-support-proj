global _keyboard_handler_stub
extern _keyboard_handler_main

_keyboard_handler_stub:
    pushad          ; Save general-purpose registers
    call _keyboard_handler_main ; Call C keyboard handler
    popad           ; Restore registers
    iret            ; Return from interrupt