#offset 0x00

.STRING msg_add_cf_of 'Math.Addition CF/OF Test: '
.STRING msg_sub_cf_of 'Math.Subtraction CF/OF Test: '
.STRING msg_mul_cf_of 'Math.Multiplication CF/OF Test: '
.STRING msg_div_cf_of 'Math.Division CF/OF Test: '

.STRING msg_zf ' ZF: '
.STRING msg_sf ' SF: '
.STRING msg_cf ' CF: '
.STRING msg_of ' OF: '
.STRING newline '\n'

; --- Function to Print Flags ---
print_flags:
    MOV F0, 0x04        ; Print String
    MOV F1, msg_zf
    INT 0x01
    MOV F0, 0x0E        ; Print Number (Float - will print 0 or 1 for flags)
    MOV F1, R10         ; ZF value in R10
    INT 0x01

    MOV F0, 0x04
    MOV F1, msg_sf
    INT 0x01
    MOV F0, 0x0E
    MOV F1, R11         ; SF value in R11
    INT 0x01

    MOV F0, 0x04
    MOV F1, msg_cf
    INT 0x01
    MOV F0, 0x0E
    MOV F1, R12         ; CF value in R12
    INT 0x01

    MOV F0, 0x04
    MOV F1, msg_of
    INT 0x01
    MOV F0, 0x0E
    MOV F1, R13         ; OF value in R13
    INT 0x01

    MOV F0, 0x05        ; Newline
    INT 0x01
    ;RET  <-- Removed RET instruction


; --- Addition CF/OF Test ---
MOV F0, 0x04
MOV F1, msg_add_cf_of
INT 0x01

MOV R0, 10.0
MOV R1, 5.0
math.add R0, R1
MOV R10, ZF         ; Get Flags
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 1.0E+38     ; Large numbers to test overflow
MOV R1, 1.0E+38
math.add R0, R1
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, -1.0E+38    ; Large negative numbers
MOV R1, -1.0E+38
math.add R0, R1
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 0.0
MOV R1, 0.0
math.add R0, R1      ; Zero result
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags


; --- Subtraction CF/OF Test ---
MOV F0, 0x04
MOV F1, msg_sub_cf_of
INT 0x01

MOV R0, 10.0
MOV R1, 5.0
math.sub R0, R1
MOV R10, ZF         ; Get Flags
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 5.0
MOV R1, 10.0
math.sub R0, R1      ; Negative result
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 1.0E+38
MOV R1, -1.0E+38    ; Subtracting a negative large number
math.sub R0, R1
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 0.0
MOV R1, 0.0
math.sub R0, R1      ; Zero result
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags


; --- Multiplication CF/OF Test ---
MOV F0, 0x04
MOV F1, msg_mul_cf_of
INT 0x01

MOV R0, 10.0
MOV R1, 5.0
math.mul R0, R1
MOV R10, ZF         ; Get Flags
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 1.0E+20     ; Multiplying large numbers
MOV R1, 1.0E+20
math.mul R0, R1
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 0.0
MOV R1, 1.0E+38
math.mul R0, R1      ; Zero result
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags


; --- Division CF/OF Test ---
MOV F0, 0x04
MOV F1, msg_div_cf_of
INT 0x01

MOV R0, 10.0
MOV R1, 5.0
math.div R0, R1
MOV R10, ZF         ; Get Flags
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 1.0E+38     ; Divide large by small
MOV R1, 1.0E-10
math.div R0, R1
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 5.0
MOV R1, 0.000001    ; Divide small by relatively larger
math.div R0, R1
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags

MOV R0, 0.0
MOV R1, 5.0
math.div R0, R1      ; Zero result
MOV R10, ZF
MOV R11, SF
MOV R12, CF
MOV R13, OF
CALL print_flags


HLT
