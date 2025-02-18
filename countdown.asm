#offset 0x00

; --- Countdown Program ---
MOV R0, 100       ; Initialize counter to 100

countdown_loop:
    sys.print_number_dec R0    ; Print counter R0
    sys.newline                ; Print newline

    sys.wait 100              ; Wait for 100 milliseconds

    MOV R1, 1           ; Value to subtract
    math.sub R0, R1     ; Decrement counter R0 by 1

    CMP R0, 0           ; Compare counter R0 with 0
    JMP_NZ countdown_loop ; Jump back to countdown_loop if Not Zero (R0 != 0)

HLT                 ; Halt CPU when countdown reaches 0
