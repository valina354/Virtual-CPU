; example.asm
#define PRINT_CHAR 0x01
#define PRINT_STRING 0x04
#define PRINT_NEWLINE 0x05
#define CLEAR_SCREEN 0x02
#define WAIT 0x03

#define READ_CHAR 0x06
#define READ_STRING 0x07
#define PRINT_NUMBER_DEC 0x08
#define PRINT_NUMBER_HEX 0x09
#define SET_CURSOR_POS 0x0A
#define GET_CURSOR_POS 0x0B
#define SET_TEXT_COLOR 0x0C
#define RESET_TEXT_COLOR 0x0D

.STRING helloworldmsg 'Hello world!'
.STRING input_prompt 'Enter your name: '
.STRING name_buffer  '                    ' ; 20 spaces for name buffer

MOV R0, SET_TEXT_COLOR  ; Set text color to Green (ANSI code 2)
MOV R1, 2
INT 0x02

MOV R0, CLEAR_SCREEN    ; Clear screen
INT 0x01

MOV R0, PRINT_STRING    ; Print "Hello world!"
MOV R1, helloworldmsg
INT 0x01

MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01

MOV R0, SET_TEXT_COLOR  ; Set text color to Yellow (ANSI code 6)
MOV R1, 6
INT 0x02

MOV R0, PRINT_STRING    ; Print prompt
MOV R1, input_prompt
INT 0x01

MOV R0, RESET_TEXT_COLOR ; Reset to default color
INT 0x02

MOV R0, READ_STRING     ; Read string input to name_buffer, max 20 chars
MOV R1, name_buffer
MOV R2, 20
INT 0x02

MOV R0, SET_TEXT_COLOR  ; Set text color to Cyan (ANSI code 3)
MOV R1, 3
INT 0x02

MOV R0, PRINT_STRING    ; Print "Hello, "
MOV R1, helloworldmsg ; Reusing helloworld for "Hello, " prefix - fix later for better example
INT 0x01

MOV R0, PRINT_STRING    ; Print the entered name
MOV R1, name_buffer
INT 0x01

MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01

MOV R0, SET_TEXT_COLOR  ; Set text color to Magenta (ANSI code 5)
MOV R1, 5
INT 0x02

MOV R0, PRINT_NUMBER_DEC ; Print a number in decimal (e.g., 12345)
MOV R1, 12345
INT 0x02

MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01

MOV R0, PRINT_NUMBER_HEX ; Print a number in hexadecimal (e.g., 0xABCDEF)
MOV R1, 0xABCDEF
INT 0x02

MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01

MOV R0, SET_CURSOR_POS  ; Set cursor position to row 5, column 10 (0-indexed)
MOV R1, 10
MOV R2, 5
INT 0x02

MOV R0, PRINT_STRING    ; Print "Cursor here" at the set position
MOV R1, helloworldmsg ; Reusing helloworld again - fix later for better example
INT 0x01

MOV R0, GET_CURSOR_POS  ; Get current cursor position
INT 0x02
MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01
MOV R0, PRINT_STRING    ; Print "Cursor X: "
MOV R1, helloworldmsg ; Reusing helloworld again - fix later for better example
INT 0x01
MOV R0, PRINT_NUMBER_DEC ; Print Cursor X
MOV R1, R1
INT 0x02
MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01
MOV R0, PRINT_STRING    ; Print "Cursor Y: "
MOV R1, helloworldmsg ; Reusing helloworld again - fix later for better example
INT 0x01
MOV R0, PRINT_NUMBER_DEC ; Print Cursor Y
MOV R1, R2
INT 0x02
MOV R0, PRINT_NEWLINE   ; Newline
INT 0x01

MOV R0, RESET_TEXT_COLOR ; Reset color to default
INT 0x02

HLT                     ; Halt
