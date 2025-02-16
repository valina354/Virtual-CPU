#offset 0x00

.STRING msg_neg_result 'Negation Result: '

; --- Main Program ---
start:
    MOV F0, 0x04        ; BIOS Print String function
    MOV F1, msg_neg_result
    INT 0x01

    MOV R0, 4.5         ; Load 4.5 into R0
    math.neg R0          ; Negate R0

    MOV F0, 0x0E        ; BIOS Print Number (Float) function
    MOV F1, R0          ; Value to print (R0 after negation)
    INT 0x01

    HLT
