#offset 0x00

; --- Program to draw a horizontal line across the screen ---

start:
    gfx.init           ; Initialize graphics

    mov R2, 1          ; Set color to white (palette index 1)
    mov R1, 64         ; Set Y coordinate for the line (middle of screen = 128/2 = 64)
    mov R0, 0          ; Initialize X coordinate to 0

draw_line_loop:
    gfx.pixel R0, R1, R2 ; Draw a pixel at (R0, R1) with color R2

    inc R0             ; Increment X coordinate (move to the next pixel horizontally)
    cmp R0, 128        ; Compare R0 (X coordinate) with 128 (SCREEN_WIDTH)
    jl draw_line_loop  ; Jump if Less than 128 (continue the loop if X < 128)

finish:
    hlt                ; Halt execution
