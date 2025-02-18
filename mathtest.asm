#offset 0x00

.STRING msg_neg_result 'Negation Result: '

; --- Main Program ---
start:
    MOV R0, msg_neg_result  ; Load address of "Negation Result: " string into R0
    sys.print_string R0      ; Print the string "Negation Result: "

    MOV R0, 4.5             ; Load 4.5 into R0
    math.neg R0              ; Negate R0

    sys.print_number_dec R0 ; Print the negated number (float) in R0

    sys.newline             ; Print a newline character for better formatting
    HLT                     ; Halt CPU
