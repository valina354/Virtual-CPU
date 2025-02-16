#offset 0x00

; --- Countdown Program ---
MOV R0, 100       ; Initialize counter to 100

countdown_loop:
    MOV F0, 0x0E        ; BIOS Function: PRINT_NUMBER_DEC
    MOV F1, R0          ; Number to print (from counter register R0)
    INT 0x01            ; Call BIOS Video Interrupt

    MOV F0, 0x05        ; BIOS Function: PRINT_NEWLINE
    INT 0x01            ; Call BIOS Video Interrupt

    MOV F0, 0x03        ; BIOS Function: WAIT
    MOV F1, 100         ; Wait for 100 milliseconds
    INT 0x02            ; Call BIOS System Interrupt

    MOV R1, 1           ; Value to subtract
    math.sub R0, R1     ; Decrement counter R0 by 1

    CMP R0, 0           ; Compare counter R0 with 0
    JMP_NZ countdown_loop ; Jump back to countdown_loop if Not Zero (R0 != 0)

HLT                 ; Halt CPU when countdown reaches 0
