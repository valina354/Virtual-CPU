#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <fcntl.h>
#include <limits.h> // Required for INT_MAX and INT_MIN
#endif
#include <time.h>

#define MEMORY_SIZE (16384 * 1024) // 16MB Memory
#define NUM_GENERAL_REGISTERS 32
#define NUM_F_REGISTERS 4
#define CPU_VER 3 // CPU Version 3 (Buffer & String Lib Support)
#define MAX_PREPROCESSOR_DEPTH 10 // Maximum depth for nested preprocessor directives

// CPU Speed Simulation
#define CLOCK_SPEED_MHZ 4.77
#define INSTRUCTIONS_PER_SECOND (CLOCK_SPEED_MHZ * 1000000)
#define TARGET_INSTRUCTIONS_PER_SECOND 100000
#define DELAY_DURATION_US (1000000 / TARGET_INSTRUCTIONS_PER_SECOND)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif


// Opcodes
typedef enum {
    OP_NOP,          // No Operation
    OP_MOV_REG_VAL,  // Move Value to Register
    OP_MOV_REG_REG,  // Move Register to Register
    OP_MOV_REG_MEM,  // Move Memory to Register
    OP_MOV_MEM_REG,  // Move Register to Memory
    OP_ADD_REG_REG,  // Add Register to Register
    OP_ADD_REG_VAL,  // Add Value to Register
    OP_SUB_REG_REG,  // Subtract Register from Register
    OP_SUB_REG_VAL,  // Subtract Value from Register
    OP_MUL_REG_REG,  // Multiply Register by Register
    OP_MUL_REG_VAL,  // Multiply Value to Register
    OP_DIV_REG_REG,  // Divide Register by Register
    OP_DIV_REG_VAL,  // Divide Value to Register
    OP_MOD_REG_REG,  // Modulo Register by Register
    OP_MOD_REG_VAL,  // Modulo Value to Register
    OP_AND_REG_REG,  // Bitwise AND Register with Register
    OP_AND_REG_VAL,  // Bitwise AND Value to Register
    OP_OR_REG_REG,   // Bitwise OR Register with Register
    OP_OR_REG_VAL,   // Bitwise OR Value to Register
    OP_XOR_REG_REG,  // Bitwise XOR Register with Register
    OP_XOR_REG_VAL,  // Bitwise XOR Value to Register
    OP_NOT_REG,      // Bitwise NOT Register
    OP_NEG_REG,      // Negate Register
    OP_CMP_REG_REG,  // Compare Register with Register
    OP_CMP_REG_VAL,  // Compare Register with Value
    OP_TEST_REG_REG, // Bitwise AND Register with Register, update flags
    OP_TEST_REG_VAL, // Bitwise AND Value, update flags
    OP_IMUL_REG_REG, // Integer Multiply Register by Register (signed)
    OP_IDIV_REG_REG, // Integer Divide Register by Register (signed)
    OP_MOVZX_REG_REG, // Move with Zero-Extend Register to Register
    OP_MOVZX_REG_MEM, // Move with Zero-Extend Memory to Register
    OP_MOVSX_REG_REG, // Move with Sign-Extend Register to Register
    OP_MOVSX_REG_MEM, // Move with Sign-Extend Memory to Register
    OP_LEA_REG_MEM,   // Load Effective Address
    OP_JMP,          // Jump Unconditional
    OP_JMP_NZ,       // Jump if Not Zero
    OP_JMP_Z,        // Jump if Zero
    OP_JMP_S,        // Jump if Sign (negative)
    OP_JMP_NS,       // Jump if Not Sign (non-negative)
    OP_JMP_C,        // Jump if Carry
    OP_JMP_NC,       // Jump if Not Carry
    OP_JMP_O,        // Jump if Overflow
    OP_JMP_NO,       // Jump if Not Overflow
    OP_JMP_GE,      // Jump if greater or equal (signed)
    OP_JMP_LE,      // Jump if less or equal (signed)
    OP_JMP_G,       // Jump if greater (signed)
    OP_JMP_L,       // Jump if less (signed)
    OP_HLT,          // Halt CPU
    OP_INC_REG,      // Increment Register
    OP_DEC_REG,      // Decrement Register
    OP_INC_MEM,      // Increment Memory
    OP_DEC_MEM,      // Decrement Memory
    OP_SHL_REG_REG,  // Shift Left Register by Register
    OP_SHL_REG_VAL,  // Shift Left Register by Value
    OP_SHR_REG_REG,  // Shift Right Register by Register (logical)
    OP_SHR_REG_VAL,  // Shift Right Register by Value (logical)
    OP_SAR_REG_REG,  // Shift Right Register by Register (arithmetic)
    OP_SAR_REG_VAL,  // Shift Right Register by Value (arithmetic)
    OP_ROL_REG_REG,  // Rotate Left Register by Register
    OP_ROL_REG_VAL,  // Rotate Left Register by Value
    OP_ROR_REG_REG,  // Rotate Right Register by Register
    OP_ROR_REG_VAL,  // Rotate Right Register by Value
    OP_RND_REG,      // Generate Random Number in Register
    OP_PUSH_REG,     // Push Register onto Stack
    OP_POP_REG,      // Pop Register from Stack
    OP_CALL_ADDR,    // Call Subroutine
    OP_RET,          // Return from Subroutine

    // Math Standard Library Opcodes
    OP_MATH_ADD,
    OP_MATH_SUB,
    OP_MATH_MUL,
    OP_MATH_DIV,
    OP_MATH_MOD,
    OP_MATH_ABS,
    OP_MATH_SIN,
    OP_MATH_COS,
    OP_MATH_TAN,
    OP_MATH_ASIN,
    OP_MATH_ACOS,
    OP_MATH_ATAN,
    OP_MATH_POW,
    OP_MATH_SQRT,
    OP_MATH_LOG,
    OP_MATH_EXP,
    OP_MATH_FLOOR,
    OP_MATH_CEIL,
    OP_MATH_ROUND,
    OP_MATH_MIN,
    OP_MATH_MAX,
    OP_MATH_NEG,

    // String Standard Library Opcodes
    OP_STR_LEN_REG_MEM,
    OP_STR_CPY_MEM_MEM,
    OP_STR_CAT_MEM_MEM,
    OP_STR_CMP_REG_MEM_MEM,
    OP_STR_NCPY_MEM_MEM_REG,
    OP_STR_NCAT_MEM_MEM_REG,
    OP_STR_TOUPPER_MEM,
    OP_STR_TOLOWER_MEM,
    OP_STR_CHR_REG_MEM_VAL,
    OP_STR_STR_REG_MEM_MEM,

    // Memory Standard Library Opcodes
    OP_MEM_CPY_MEM_MEM_REG,
    OP_MEM_SET_MEM_REG_VAL,
    OP_MEM_FREE_MEM,

    // System Standard Library Opcodes (Replaces BIOS Interrupts)
    OP_SYS_PRINT_CHAR,
    OP_SYS_CLEAR_SCREEN,
    OP_SYS_PRINT_STRING,
    OP_SYS_PRINT_NEWLINE,
    OP_SYS_SET_CURSOR_POS,
    OP_SYS_GET_CURSOR_POS,
    OP_SYS_SET_TEXT_COLOR,
    OP_SYS_RESET_TEXT_COLOR,
    OP_SYS_PRINT_NUMBER_DEC,
    OP_SYS_PRINT_NUMBER_HEX,
    OP_SYS_NUMBER_TO_STRING,
    OP_SYS_READ_CHAR,
    OP_SYS_READ_STRING,
    OP_SYS_GET_KEY_PRESS,
    OP_SYS_GET_CPU_VER,
    OP_SYS_WAIT,

    OP_INVALID       // Invalid Opcode
} Opcode;

// Registers
typedef enum {
    REG_R0, REG_R1, REG_R2, REG_R3, REG_R4, REG_R5, REG_R6, REG_R7,
    REG_R8, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15,
    REG_R16, REG_R17, REG_R18, REG_R19, REG_R20, REG_R21, REG_R22, REG_R23,
    REG_R24, REG_R25, REG_R26, REG_R27, REG_R28, REG_R29, REG_R30, REG_R31, // General purpose registers
    REG_SP,                                                                  // Stack Pointer Register
    REG_F0, REG_F1, REG_F2, REG_F3,                                          // Function Registers (for BIOS calls) - now general purpose
    REG_ZF, // Zero Flag
    REG_SF, // Sign Flag
    REG_CF, // Carry Flag
    REG_OF, // Overflow Flag
    REG_FLAG_COUNT, // Keep track of total flag registers for bounds checking
    REG_INVALID
} RegisterIndex;

#define NUM_FLAG_REGISTERS (REG_FLAG_COUNT - REG_F3 - 1 - 2) // Calculate number of flag registers - adjusted, now including CF, OF
#define NUM_TOTAL_REGISTERS (NUM_GENERAL_REGISTERS + NUM_F_REGISTERS + NUM_FLAG_REGISTERS) // Update total registers


// Macro Definition Structure
typedef struct {
    char name[32];
    char value_str[32];
} MacroDefinition;

// Label Definition Structure
typedef struct {
    char name[32];
    uint32_t address;
} LabelDefinition;

// String Definition Structure
typedef struct {
    char name[32];
    uint32_t address;
    char value[256];
} StringDefinition;

// Buffer Definition Structure
typedef struct {
    char name[32];
    uint32_t address;
    uint32_t size;
} BufferDefinition;


uint8_t memory[MEMORY_SIZE];                  // CPU Memory
double registers[NUM_TOTAL_REGISTERS];       // CPU Registers
uint32_t program_counter = 0;                 // Program Counter
bool running = true;                           // CPU Running Flag
bool zero_flag = false;                        // Zero Flag
bool sign_flag = false;                        // Sign Flag
bool carry_flag = false;                       // Carry Flag
bool overflow_flag = false;                    // Overflow Flag
MacroDefinition macros[1024];                  // Macro Definitions
int macro_count = 0;                           // Macro Count
LabelDefinition labels[1024];                  // Label Definitions
int label_count = 0;                           // Label Count
StringDefinition strings[1024];                 // String Definitions
int string_count = 0;                          // String Count
BufferDefinition buffers[1024];                 // Buffer Definitions
int buffer_count = 0;                          // Buffer Count
uint32_t data_section_start = 0;              // Start of Data Section in Memory

int cursor_x = 0;                             // Cursor X Position
int cursor_y = 0;                             // Cursor Y Position
int text_color = 7;                           // Default Text Color (White on Black)

#ifndef _WIN32
struct termios original_termios;

// Enable Raw Mode for Non-Blocking Input (Linux)
void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Disable Raw Mode (Linux)
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}
#endif

// System Library Functions (replacing BIOS)

// SYS Function: Read a Character from Input
char sys_read_char() {
#ifdef _WIN32
    return _getch();
#else
    enable_raw_mode();
    char c = getchar();
    disable_raw_mode();
    return c;
#endif
}

// SYS Function: Get Key Press without Blocking (returns 0 if no key pressed)
char sys_get_key_press() {
#ifdef _WIN32
    if (_kbhit()) {
        return _getch();
    }
    else {
        return 0;
    }
#else
    struct timeval tv;
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0; // Non-blocking select

    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
        enable_raw_mode();
        char c = getchar();
        disable_raw_mode();
        return c;
    }
    else {
        return 0;
    }
#endif
}

// SYS Function: Print a Character to Console
void sys_print_char(char character) {
    printf("%c", character);
}

// SYS Function: Print a Newline Character
void sys_print_newline() {
    sys_print_char('\n');
    cursor_y++;
    cursor_x = 0;
}

// SYS Function: Read a String from Input
void sys_read_string(uint32_t address, uint32_t max_len) {
    if (address >= MEMORY_SIZE) {
        printf("Error: String address out of memory bounds in READ_STRING.\n");
        return;
    }
    char* str_ptr = (char*)&memory[address];
    uint32_t i = 0;
    char c;
    while (i < max_len - 1) {
        c = sys_read_char();
        if (c == '\r' || c == '\n') {
            str_ptr[i] = '\0';
            sys_print_newline();
            break;
        }
        else if (c == 127) { // Backspace
            if (i > 0) {
                i--;
                str_ptr[i] = '\0';
                printf("\b \b"); // Erase character from console
            }
        }
        else if (c >= 32 && c <= 126) { // Printable characters
            str_ptr[i++] = c;
            sys_print_char(c);
        }
        else if (c == 27) { // Escape character - ignore
            continue;
        }
    }
    if (i == max_len - 1) {
        str_ptr[i] = '\0'; // Null-terminate if max length reached
    }
}

// SYS Function: Print Unsigned Decimal Number
void sys_print_number_dec(double number) {
    printf("%f", number);
}

// SYS Function: Print Hexadecimal Number
void sys_print_number_hex(uint32_t number) {
    printf("0x%X", number);
}

// SYS Function: Convert Number to String in Memory
void sys_number_to_string(uint32_t number, uint32_t address, uint32_t buffer_size) {
    if (address >= MEMORY_SIZE || address + buffer_size > MEMORY_SIZE) {
        printf("Error: Buffer address out of memory bounds in NUMBER_TO_STRING.\n");
        return;
    }
    char* str_ptr = (char*)&memory[address];
    snprintf(str_ptr, buffer_size, "%u", number);
}

// SYS Function: Set Cursor Position
void sys_set_cursor_pos(uint32_t x, uint32_t y) {
    cursor_x = x;
    cursor_y = y;
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, pos);
#else
    printf("\033[%d;%dH", y + 1, x + 1); // ANSI escape code to set cursor position
#endif
}

// SYS Function: Get Cursor Position (returns in F1 and F2 registers)
void sys_get_cursor_pos(uint32_t* x, uint32_t* y) {
    *x = cursor_x;
    *y = cursor_y;
    registers[REG_F1] = cursor_x;
    registers[REG_F2] = cursor_y;
}

// SYS Function: Set Text Color (64 colors - extended ANSI)
void sys_set_text_color(uint32_t color_code) {
    text_color = color_code;
#ifdef _WIN32
    // Windows basic 16 colors - map 64 to 16 (basic mapping for now)
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)(color_code % 16));
#else
    // 64 Color ANSI Escape Codes (Foreground only for simplicity)
    if (color_code >= 0 && color_code <= 63) {
        printf("\033[38;5;%dm", color_code); // ANSI escape code for 256 colors, using 64 subset
    }
    else {
        printf("\033[37m"); // Default white if out of range
    }
#endif
}


// SYS Function: Reset Text Color to Default
void sys_reset_text_color() {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Default color on Windows
#else
    printf("\033[0m"); // ANSI escape code to reset all attributes
#endif
    text_color = 7; // Reset internal color tracking
}

// SYS Function: Print Null-Terminated String from Memory
void sys_print_string(uint32_t address) {
    if (address >= MEMORY_SIZE) {
        printf("Error: String address out of memory bounds.\n");
        return;
    }
    char* str = (char*)&memory[address];
    while (*str != '\0') {
        sys_print_char(*str);
        str++;
        if ((uint32_t)(str - (char*)memory) >= MEMORY_SIZE) {
            printf("Error: String exceeds memory bounds.\n");
            return;
        }
    }
}

// SYS Function: Clear Screen
void sys_clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear"); // ANSI escape to clear screen
    printf("\033[H"); // Reset cursor to top-left (optional, clear might do this)
#endif
    cursor_x = 0;
    cursor_y = 0;
}

// SYS Function: Wait for Milliseconds
void sys_wait(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// SYS Function: Get CPU Version
uint32_t sys_get_cpu_ver() {
    return CPU_VER;
}


// Instruction Decoding: Decode Opcode from Memory
Opcode decode_opcode() {
    if (program_counter >= MEMORY_SIZE) return OP_INVALID;
    return (Opcode)memory[program_counter++];
}

// Instruction Decoding: Decode Register Index from Memory
RegisterIndex decode_register() {
    if (program_counter >= MEMORY_SIZE) return REG_INVALID;
    uint8_t reg_index = memory[program_counter++];
    if (reg_index >= NUM_TOTAL_REGISTERS) return REG_INVALID;
    return (RegisterIndex)reg_index;
}

// Instruction Decoding: Decode 64-bit Value (double) from Memory
double decode_value_double() {
    if (program_counter + 8 > MEMORY_SIZE) return 0.0;
    double value = *(double*)&memory[program_counter];
    program_counter += 8;
    return value;
}

// Instruction Decoding: Decode 32-bit Value from Memory
uint32_t decode_value_uint32() {
    if (program_counter + 4 > MEMORY_SIZE) return 0;
    uint32_t value = *(uint32_t*)&memory[program_counter];
    program_counter += 4;
    return value;
}

// Instruction Decoding: Decode Address
uint32_t decode_address() {
    return decode_value_uint32();
}

// Flag Setting Functions
void set_zero_flag_float(double result) {
    zero_flag = (fabs(result) < 1e-9);
}

void set_sign_flag_float(double result) {
    sign_flag = (result < 0.0);
}

void set_carry_flag(double result, double operand1, double operand2, Opcode opcode) {
    // Basic carry flag logic for addition and subtraction (for integer part)
    if (opcode == OP_ADD_REG_REG || opcode == OP_ADD_REG_VAL || opcode == OP_MATH_ADD) {
        carry_flag = (result > HUGE_VAL || result < -HUGE_VAL); // Very basic overflow check for doubles
    }
    else if (opcode == OP_SUB_REG_REG || opcode == OP_SUB_REG_VAL || opcode == OP_MATH_SUB) {
        carry_flag = (result > HUGE_VAL || result < -HUGE_VAL); //  Basic underflow check for doubles
    }
    else {
        carry_flag = false; // Reset for other operations
    }
}


void set_overflow_flag(double result, double operand1, double operand2, Opcode opcode) {
    overflow_flag = false; // Overflow flag not reliably implemented for double operations in this example
}

void set_zero_flag_int(uint32_t result) {
    zero_flag = (result == 0);
}

void set_sign_flag_int(uint32_t result) {
    sign_flag = ((int32_t)result < 0); // Treat as signed int for sign flag
}

void set_carry_flag_int(uint32_t result, uint32_t operand1, uint32_t operand2, Opcode opcode) {
    if (opcode == OP_ADD_REG_REG || opcode == OP_ADD_REG_VAL) {
        carry_flag = (result < operand1); // Carry if result is smaller than operand1 (unsigned overflow)
    }
    else if (opcode == OP_SUB_REG_REG || opcode == OP_SUB_REG_VAL) {
        carry_flag = (result > operand1); // Borrow if result is larger than operand1 (unsigned underflow)
    }
    else if (opcode == OP_SHL_REG_REG || opcode == OP_SHL_REG_VAL || opcode == OP_SAR_REG_REG || opcode == OP_SAR_REG_VAL || opcode == OP_SHR_REG_REG || opcode == OP_SHR_REG_VAL || opcode == OP_ROL_REG_REG || opcode == OP_ROL_REG_VAL || opcode == OP_ROR_REG_REG || opcode == OP_ROR_REG_VAL) {
        carry_flag = (result != result); // Placeholder - for shifts and rotates, set to indicate shift happened (can be improved)
    }
    else {
        carry_flag = false;
    }
}


void set_overflow_flag_int(uint32_t result, uint32_t operand1, uint32_t operand2, Opcode opcode) {
    if (opcode == OP_ADD_REG_REG || opcode == OP_ADD_REG_VAL) {
        // Signed overflow for addition: if signs of operands are same, and sign of result is different
        overflow_flag = (((operand1 ^ operand2) & 0x80000000) == 0 && ((operand1 ^ result) & 0x80000000) != 0);
    }
    else if (opcode == OP_SUB_REG_REG || opcode == OP_SUB_REG_VAL) {
        // Signed overflow for subtraction: if signs of operands are different, and sign of result is different from first operand
        overflow_flag = (((operand1 ^ operand2) & 0x80000000) != 0 && ((operand1 ^ result) & 0x80000000) != 0);
    }
    else {
        overflow_flag = false;
    }
}


// CPU Instruction Execution
void execute_instruction(Opcode opcode) {
    RegisterIndex reg1, reg2, reg3, reg_dest, reg_src; // Added reg3
    double value_double;
    uint32_t value_uint32, address, count; // Added count

    switch (opcode) {
    case OP_NOP:
        break;
    case OP_MOV_REG_REG:
        reg_dest = decode_register();
        reg_src = decode_register();
        if (reg_dest != REG_INVALID && reg_src != REG_INVALID) {
            double src_value;
            if (reg_src == REG_ZF) src_value = zero_flag;
            else if (reg_src == REG_SF) src_value = sign_flag;
            else if (reg_src == REG_CF) src_value = carry_flag;
            else if (reg_src == REG_OF) src_value = overflow_flag;
            else src_value = registers[reg_src];

            registers[reg_dest] = src_value;
        }
        break;

    case OP_MOV_REG_VAL:
        reg_dest = decode_register();
        value_double = decode_value_double();
        if (reg_dest != REG_INVALID) {
            if (reg_dest == REG_ZF || reg_dest == REG_SF || reg_dest == REG_CF || reg_dest == REG_OF) {
                // do nothing - cannot directly write to flag registers with MOV REG_VAL
            }
            else {
                registers[reg_dest] = value_double;
            }
        }
        break;
    case OP_MOV_REG_MEM:
        reg1 = decode_register();
        address = decode_address();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE - 8) registers[reg1] = *(double*)&memory[address];
        break;
    case OP_MOV_MEM_REG:
        address = decode_address();
        reg1 = decode_register();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE - 8) *(double*)&memory[address] = registers[reg1];
        break;
    case OP_ADD_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] + registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_ADD_REG_VAL:
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double result = registers[reg1] + value_double;
            set_carry_flag(result, registers[reg1], value_double, opcode);
            set_overflow_flag(result, registers[reg1], value_double, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_SUB_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] - registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_SUB_REG_VAL:
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double result = registers[reg1] - value_double;
            set_carry_flag(result, registers[reg1], value_double, opcode);
            set_overflow_flag(result, registers[reg1], value_double, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_MUL_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] * registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_MUL_REG_VAL:
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double result = registers[reg1] * value_double;
            set_carry_flag(result, registers[reg1], value_double, opcode);
            set_overflow_flag(result, registers[reg1], value_double, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_DIV_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = registers[reg1] / registers[reg2];
                set_carry_flag(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Division by zero!\n");
                running = false;
            }
        }
        break;
    case OP_DIV_REG_VAL:
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            if (fabs(value_double) > 1e-9) {
                double result = registers[reg1] / value_double;
                set_carry_flag(result, registers[reg1], value_double, opcode);
                set_overflow_flag(result, registers[reg1], value_double, opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Division by zero!\n");
                running = false;
            }
        }
        break;
    case OP_MOD_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = fmod(registers[reg1], registers[reg2]);
                set_carry_flag(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Modulo by zero!\n");
                running = false;
            }
        }
        break;
    case OP_MOD_REG_VAL:
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            if (fabs(value_double) > 1e-9) {
                double result = fmod(registers[reg1], value_double);
                set_carry_flag(result, registers[reg1], value_double, opcode);
                set_overflow_flag(result, registers[reg1], value_double, opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Modulo by zero!\n");
                running = false;
            }
        }
        break;
    case OP_AND_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = (uint32_t)registers[reg2];
            uint32_t result = val1 & val2;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_AND_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32(); // Decode value as uint32_t for bitwise
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t result = val1 & value_uint32;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, value_uint32, opcode);
            set_overflow_flag_int(result, val1, value_uint32, opcode);
        }
        break;
    }
    case OP_OR_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = (uint32_t)registers[reg2];
            uint32_t result = val1 | val2;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_OR_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32(); // Decode value as uint32_t for bitwise
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t result = val1 | value_uint32;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, value_uint32, opcode);
            set_overflow_flag_int(result, val1, value_uint32, opcode);
        }
        break;
    }
    case OP_XOR_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = (uint32_t)registers[reg2];
            uint32_t result = val1 ^ val2;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_XOR_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32(); // Decode value as uint32_t for bitwise
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t result = val1 ^ value_uint32;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, value_uint32, opcode);
            set_overflow_flag_int(result, val1, value_uint32, opcode);
        }
        break;
    }
    case OP_NOT_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t result = ~val1;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, 0, opcode); // no carry relevant for NOT
            set_overflow_flag_int(result, val1, 0, opcode); // no overflow relevant for NOT
        }
        break;
    }
    case OP_NEG_REG:
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = -registers[reg1];
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    case OP_CMP_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double val1 = registers[reg1];
            double val2 = registers[reg2];
            set_zero_flag_float(val1 - val2);
            set_sign_flag_float(val1 - val2);
            set_carry_flag(val1 - val2, val1, val2, opcode); // Placeholder logic for carry/overflow in CMP
            set_overflow_flag(val1 - val2, val1, val2, opcode);
        }
        break;
    case OP_CMP_REG_VAL:
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double val1 = registers[reg1];
            double val2 = value_double;
            set_zero_flag_float(val1 - val2);
            set_sign_flag_float(val1 - val2);
            set_carry_flag(val1 - val2, val1, val2, opcode); // Placeholder logic for carry/overflow in CMP
            set_overflow_flag(val1 - val2, val1, val2, opcode);
        }
        break;
    case OP_TEST_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = (uint32_t)registers[reg2];
            uint32_t result = val1 & val2;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_TEST_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t result = val1 & value_uint32;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, value_uint32, opcode);
            set_overflow_flag_int(result, val1, value_uint32, opcode);
        }
        break;
    }
    case OP_IMUL_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] * registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_IDIV_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = registers[reg1] / registers[reg2];
                set_carry_flag(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Division by zero!\n");
                running = false;
            }
        }
        break;
    case OP_MOVZX_REG_REG: {
        reg_dest = decode_register();
        reg_src = decode_register();
        if (reg_dest != REG_INVALID && reg_src != REG_INVALID) {
            uint32_t src_val = (uint32_t)registers[reg_src]; // Treat source as uint32_t
            registers[reg_dest] = (double)src_val;         // Zero-extend by default when converting to double
        }
        break;
    }
    case OP_MOVZX_REG_MEM: {
        reg_dest = decode_register();
        address = decode_address();
        if (reg_dest != REG_INVALID && address < MEMORY_SIZE - 4) {
            uint32_t mem_val = *(uint32_t*)&memory[address]; // Read 32-bit from memory
            registers[reg_dest] = (double)mem_val;          // Zero-extend to double
        }
        break;
    }
    case OP_MOVSX_REG_REG: {
        reg_dest = decode_register();
        reg_src = decode_register();
        if (reg_dest != REG_INVALID && reg_src != REG_INVALID) {
            int32_t src_val = (int32_t)registers[reg_src]; // Treat source as int32_t for sign-extend
            registers[reg_dest] = (double)src_val;         // Sign-extend when converting to double
        }
        break;
    }
    case OP_MOVSX_REG_MEM: {
        reg_dest = decode_register();
        address = decode_address();
        if (reg_dest != REG_INVALID && address < MEMORY_SIZE - 4) {
            int32_t mem_val = *(int32_t*)&memory[address]; // Read 32-bit as signed from memory
            registers[reg_dest] = (double)mem_val;          // Sign-extend to double
        }
        break;
    }
    case OP_LEA_REG_MEM:
        reg_dest = decode_register();
        address = decode_address();
        if (reg_dest != REG_INVALID) {
            registers[reg_dest] = (double)address;
        }
        break;
    case OP_JMP:
        address = decode_address();
        program_counter = address;
        break;
    case OP_JMP_NZ:
        address = decode_address();
        if (!zero_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_Z:
        address = decode_address();
        if (zero_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_S:
        address = decode_address();
        if (sign_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_NS:
        address = decode_address();
        if (!sign_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_C:
        address = decode_address();
        if (carry_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_NC:
        address = decode_address();
        if (!carry_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_O:
        address = decode_address();
        if (overflow_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_NO:
        address = decode_address();
        if (!overflow_flag) {
            program_counter = address;
        }
        break;

    case OP_JMP_GE: // Jump if greater or equal (signed)
        address = decode_address();
        if (sign_flag == false) {
            program_counter = address;
        }
        break;
    case OP_JMP_LE: // Jump if less or equal (signed)
        address = decode_address();
        if (zero_flag || sign_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_G:  // Jump if greater (signed)
        address = decode_address();
        if (!zero_flag && !sign_flag) {
            program_counter = address;
        }
        break;
    case OP_JMP_L:  // Jump if less (signed)
        address = decode_address();
        if (!zero_flag && sign_flag) {
            program_counter = address;
        }
        break;
    case OP_HLT:
        running = false;
        break;
    case OP_INC_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1]++;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_DEC_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1]--;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_INC_MEM: {
        address = decode_address();
        if (address < MEMORY_SIZE - 8) {
            double val = *(double*)&memory[address];
            val++;
            *(double*)&memory[address] = val;
        }
        break;
    }
    case OP_DEC_MEM: {
        address = decode_address();
        if (address < MEMORY_SIZE - 8) {
            double val = *(double*)&memory[address];
            val--;
            *(double*)&memory[address] = val;
        }
        break;
    }
    case OP_SHL_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = (uint32_t)registers[reg2];
            uint32_t result = val1 << (shift_amount & 0x1F); // Ensure shift amount is within 0-31 bits
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after shift (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for shifts
        }
        break;
    }
    case OP_SHL_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = value_uint32;
            uint32_t result = val1 << (shift_amount & 0x1F); // Ensure shift amount is within 0-31 bits
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after shift (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for shifts
        }
        break;
    }
    case OP_SHR_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = (uint32_t)registers[reg2];
            uint32_t result = val1 >> (shift_amount & 0x1F); // Logical shift
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after shift (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for shifts
        }
        break;
    }
    case OP_SHR_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = value_uint32;
            uint32_t result = val1 >> (shift_amount & 0x1F); // Logical shift
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after shift (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for shifts
        }
        break;
    }
    case OP_SAR_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            int32_t val1 = (int32_t)registers[reg1]; // Treat as signed for SAR
            uint32_t shift_amount = (uint32_t)registers[reg2];
            int32_t result = val1 >> (shift_amount & 0x1F); // Arithmetic shift
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after shift (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for shifts
        }
        break;
    }
    case OP_SAR_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            int32_t val1 = (int32_t)registers[reg1]; // Treat as signed for SAR
            uint32_t shift_amount = value_uint32;
            int32_t result = val1 >> (shift_amount & 0x1F); // Arithmetic shift
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after shift (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for shifts
        }
        break;
    }
    case OP_ROL_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = (uint32_t)registers[reg2];
            uint32_t bits = shift_amount & 0x1F; // Effective rotation bits (0-31)
            uint32_t result = (val1 << bits) | (val1 >> (32 - bits));
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after rotation (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for rotations
        }
        break;
    }
    case OP_ROL_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = value_uint32;
            uint32_t bits = shift_amount & 0x1F; // Effective rotation bits (0-31)
            uint32_t result = (val1 << bits) | (val1 >> (32 - bits));
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after rotation (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for rotations
        }
        break;
    }
    case OP_ROR_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = (uint32_t)registers[reg2];
            uint32_t bits = shift_amount & 0x1F; // Effective rotation bits (0-31)
            uint32_t result = (val1 >> bits) | (val1 << (32 - bits));
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after rotation (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for rotations
        }
        break;
    }
    case OP_ROR_REG_VAL: {
        reg1 = decode_register();
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t shift_amount = value_uint32;
            uint32_t bits = shift_amount & 0x1F; // Effective rotation bits (0-31)
            uint32_t result = (val1 >> bits) | (val1 << (32 - bits));
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, shift_amount, opcode); // Carry flag after rotation (can be refined)
            set_overflow_flag_int(result, val1, shift_amount, opcode); // Overflow flag not typically meaningful for rotations
        }
        break;
    }

    case OP_RND_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = (double)rand();
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_PUSH_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[REG_SP] -= 8;
            if ((int32_t)registers[REG_SP] < 0) {
                printf("Stack Overflow!\n");
                running = false;
                break;
            }
            *(double*)&memory[(uint32_t)registers[REG_SP]] = registers[reg1];
        }
        break;
    }
    case OP_POP_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            if ((uint32_t)registers[REG_SP] >= MEMORY_SIZE) {
                printf("Stack Underflow!\n");
                running = false;
                break;
            }
            registers[reg1] = *(double*)&memory[(uint32_t)registers[REG_SP]];
            registers[REG_SP] += 8;
        }
        break;
    }
    case OP_CALL_ADDR: {
        address = decode_address();
        registers[REG_SP] -= 8;
        if ((int32_t)registers[REG_SP] < 0) {
            printf("Stack Overflow during CALL!\n");
            running = false;
            break;
        }
        *(double*)&memory[(uint32_t)registers[REG_SP]] = (double)program_counter;
        program_counter = address;
        break;
    }
    case OP_RET: {
        if ((uint32_t)registers[REG_SP] >= MEMORY_SIZE) {
            printf("Stack Underflow during RET!\n");
            running = false;
            break;
        }
        program_counter = (uint32_t) * (double*)&memory[(uint32_t)registers[REG_SP]];
        registers[REG_SP] += 8;
        break;
    }
               // Math Standard Library Implementation
    case OP_MATH_ADD:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] + registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_MATH_SUB:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] - registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_MATH_MUL:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] * registers[reg2];
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    case OP_MATH_DIV:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = registers[reg1] / registers[reg2];
                set_carry_flag(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Math Error: Division by zero!\n");
                running = false;
            }
        }
        break;
    case OP_MATH_MOD:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = fmod(registers[reg1], registers[reg2]);
                set_carry_flag(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Math Error: Modulo by zero!\n");
                running = false;
            }
        }
        break;
    case OP_MATH_ABS:
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = fabs(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    case OP_MATH_SIN: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = sin(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_COS: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = cos(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_TAN: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = tan(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_ASIN: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = asin(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_ACOS: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = acos(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_ATAN: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = atan(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_POW: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = pow(registers[reg1], registers[reg2]);
            set_carry_flag(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_MATH_SQRT: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            if (registers[reg1] >= 0) {
                registers[reg1] = sqrt(registers[reg1]);
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
                set_carry_flag(registers[reg1], 0, 0, opcode);
                set_overflow_flag(registers[reg1], 0, 0, opcode);
            }
            else {
                printf("Math Error: Square root of negative number!\n");
                running = false;
            }
        }
        break;
    }
    case OP_MATH_LOG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            if (registers[reg1] > 0) {
                registers[reg1] = log(registers[reg1]);
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
                set_carry_flag(registers[reg1], 0, 0, opcode);
                set_overflow_flag(registers[reg1], 0, 0, opcode);
            }
            else {
                printf("Math Error: Logarithm of non-positive number!\n");
                running = false;
            }
        }
        break;
    }
    case OP_MATH_EXP: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = exp(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_FLOOR: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = floor(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_CEIL: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = ceil(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_ROUND: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = round(registers[reg1]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_MATH_MIN:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] = (registers[reg1] < registers[reg2]) ? registers[reg1] : registers[reg2];
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], registers[reg1], registers[reg2], opcode);
            set_overflow_flag(registers[reg1], registers[reg1], registers[reg2], opcode);
        }
        break;
    case OP_MATH_MAX:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] = (registers[reg1] > registers[reg2]) ? registers[reg1] : registers[reg2];
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], registers[reg1], registers[reg2], opcode);
            set_overflow_flag(registers[reg1], registers[reg1], registers[reg2], opcode);
        }
        break;
    case OP_MATH_NEG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = -registers[reg1];
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag(registers[reg1], 0, 0, opcode);
            set_overflow_flag(registers[reg1], 0, 0, opcode);
        }
        break;
    }
                  // String Standard Library Implementation
    case OP_STR_LEN_REG_MEM: {
        reg1 = decode_register();
        address = decode_address();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE) {
            char* str = (char*)&memory[address];
            registers[reg1] = (double)strlen(str);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_STR_CPY_MEM_MEM: {
        address = decode_address(); // Destination address
        uint32_t src_address = decode_address(); // Source address
        if (address < MEMORY_SIZE && src_address < MEMORY_SIZE) {
            char* dest_str = (char*)&memory[address];
            char* src_str = (char*)&memory[src_address];
            strcpy(dest_str, src_str); // Potential buffer overflow if dest is too small - needs bounds checking in real CPU
        }
        break;
    }
    case OP_STR_CAT_MEM_MEM: {
        address = decode_address(); // Destination address
        uint32_t src_address = decode_address(); // Source address
        if (address < MEMORY_SIZE && src_address < MEMORY_SIZE) {
            char* dest_str = (char*)&memory[address];
            char* src_str = (char*)&memory[src_address];
            strcat(dest_str, src_str); // Potential buffer overflow if dest is too small - needs bounds checking in real CPU
        }
        break;
    }
    case OP_STR_CMP_REG_MEM_MEM: {
        reg1 = decode_register();
        uint32_t addr1 = decode_address();
        uint32_t addr2 = decode_address();
        if (reg1 != REG_INVALID && addr1 < MEMORY_SIZE && addr2 < MEMORY_SIZE) {
            char* str1 = (char*)&memory[addr1];
            char* str2 = (char*)&memory[addr2];
            registers[reg1] = (double)strcmp(str1, str2);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_STR_NCPY_MEM_MEM_REG: {
        address = decode_address(); // Destination address
        uint32_t src_address = decode_address(); // Source address
        reg1 = decode_register(); // Number of bytes to copy from register
        if (address < MEMORY_SIZE && src_address < MEMORY_SIZE && reg1 != REG_INVALID) {
            char* dest_str = (char*)&memory[address];
            char* src_str = (char*)&memory[src_address];
            count = (uint32_t)registers[reg1];
            strncpy(dest_str, src_str, count);
            dest_str[count] = '\0'; // Ensure null termination
        }
        break;
    }
    case OP_STR_NCAT_MEM_MEM_REG: {
        address = decode_address(); // Destination address
        uint32_t src_address = decode_address(); // Source address
        reg1 = decode_register(); // Number of bytes to append from register
        if (address < MEMORY_SIZE && src_address < MEMORY_SIZE && reg1 != REG_INVALID) {
            char* dest_str = (char*)&memory[address];
            char* src_str = (char*)&memory[src_address];
            count = (uint32_t)registers[reg1];
            strncat(dest_str, src_str, count);
        }
        break;
    }
    case OP_MEM_CPY_MEM_MEM_REG: {
        uint32_t dest_address = decode_address();
        uint32_t src_address = decode_address();
        reg1 = decode_register(); // Count register
        if (dest_address < MEMORY_SIZE && src_address < MEMORY_SIZE && reg1 != REG_INVALID) {
            uint32_t count = (uint32_t)registers[reg1];
            if (dest_address + count > MEMORY_SIZE || src_address + count > MEMORY_SIZE) {
                printf("Error: MEMCPY out of memory bounds!\n");
                running = false;
                break;
            }
            memcpy(&memory[dest_address], &memory[src_address], count);
        }
        break;
    }
    case OP_MEM_SET_MEM_REG_VAL: {
        uint32_t dest_address = decode_address();
        reg1 = decode_register(); // Value register
        value_uint32 = decode_value_uint32(); // Count value (immediate)
        if (dest_address < MEMORY_SIZE && reg1 != REG_INVALID) {
            uint8_t value = (uint8_t)registers[reg1]; // Treat value as a byte
            if (dest_address + value_uint32 > MEMORY_SIZE) {
                printf("Error: MEMSET out of memory bounds!\n");
                running = false;
                break;
            }
            memset(&memory[dest_address], value, value_uint32);
        }
        break;
    }
    case OP_MEM_FREE_MEM: { // <--- ADD THIS CASE
        uint32_t address = decode_address();
        if (address < MEMORY_SIZE) {
            // Get buffer definition to know the size
            uint32_t buffer_size = 0;
            for (int i = 0; i < buffer_count; i++) {
                if (buffers[i].address == address) {
                    buffer_size = buffers[i].size;
                    break;
                }
            }
            if (buffer_size > 0) {
                memset(&memory[address], 0, buffer_size); // Clear the buffer with null bytes
            }
            else {
                printf("Warning: mem.free called on address without known buffer size.\n");
            }
        }
        else {
            printf("Error: MEMFREE address out of memory bounds!\n");
            running = false;
        }
        break;
    }
    case OP_STR_TOUPPER_MEM: {
        address = decode_address();
        if (address < MEMORY_SIZE) {
            char* str = (char*)&memory[address];
            char* p = str;
            while (*p) {
                *p = toupper((unsigned char)*p);
                p++;
            }
        }
        else {
            printf("Error: STRTOUPPER address out of memory bounds!\n");
            running = false;
        }
        break;
    }
    case OP_STR_TOLOWER_MEM: {
        address = decode_address();
        if (address < MEMORY_SIZE) {
            char* str = (char*)&memory[address];
            char* p = str;
            while (*p) {
                *p = tolower((unsigned char)*p);
                p++;
            }
        }
        else {
            printf("Error: STRTOLOWER address out of memory bounds!\n");
            running = false;
        }
        break;
    }
    case OP_STR_CHR_REG_MEM_VAL: {
        reg1 = decode_register();
        address = decode_address();
        uint32_t char_val = decode_value_uint32(); // Character value is decoded as uint32_t
        if (reg1 != REG_INVALID && address < MEMORY_SIZE) {
            char* str = (char*)&memory[address];
            char char_to_find = (char)char_val; // Cast to char for comparison
            char* result = strchr(str, char_to_find);
            if (result) {
                registers[reg1] = (double)(result - str); // Store index in register
            }
            else {
                registers[reg1] = -1.0; // Not found, store -1
            }
            set_zero_flag_float(registers[reg1]); // Update flags based on result
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_STR_STR_REG_MEM_MEM: { // <--- ADD THIS CASE
        reg1 = decode_register();
        uint32_t haystack_addr = decode_address();
        uint32_t needle_addr = decode_address();
        if (reg1 != REG_INVALID && haystack_addr < MEMORY_SIZE && needle_addr < MEMORY_SIZE) {
            char* haystack = (char*)&memory[haystack_addr];
            char* needle = (char*)&memory[needle_addr];
            char* result = strstr(haystack, needle);
            if (result) {
                registers[reg1] = (double)(result - haystack); // Store starting index
            }
            else {
                registers[reg1] = -1.0; // Not found, store -1
            }
            set_zero_flag_float(registers[reg1]); // Update flags
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
                               // System Library Opcodes Implementation
    case OP_SYS_PRINT_CHAR:
        sys_print_char((char)(uint32_t)registers[REG_R0]); // Assuming argument in R0
        cursor_x++;
        break;
    case OP_SYS_CLEAR_SCREEN:
        sys_clear_screen();
        break;
    case OP_SYS_PRINT_STRING:
        sys_print_string((uint32_t)registers[REG_R0]); // Assuming address in R0
        break;
    case OP_SYS_PRINT_NEWLINE:
        sys_print_newline();
        break;
    case OP_SYS_SET_CURSOR_POS:
        sys_set_cursor_pos((uint32_t)registers[REG_R0], (uint32_t)registers[REG_R1]); // Assuming X in R0, Y in R1
        break;
    case OP_SYS_GET_CURSOR_POS:
        sys_get_cursor_pos(NULL, NULL); // Results in F1, F2 - which are now general purpose, can use R1, R2 instead if needed.
        registers[REG_R0] = registers[REG_F1]; // Return X in R0
        registers[REG_R1] = registers[REG_F2]; // Return Y in R1
        break;
    case OP_SYS_SET_TEXT_COLOR:
        sys_set_text_color((uint32_t)registers[REG_R0]); // Assuming color code in R0
        break;
    case OP_SYS_RESET_TEXT_COLOR:
        sys_reset_text_color();
        break;
    case OP_SYS_PRINT_NUMBER_DEC:
        sys_print_number_dec(registers[REG_R0]); // Assuming number in R0
        break;
    case OP_SYS_PRINT_NUMBER_HEX:
        sys_print_number_hex((uint32_t)registers[REG_R0]); // Assuming number in R0
        break;
    case OP_SYS_NUMBER_TO_STRING:
        sys_number_to_string((uint32_t)registers[REG_R0], (uint32_t)registers[REG_R1], (uint32_t)registers[REG_R2]); // Assuming num in R0, addr in R1, size in R2
        break;
    case OP_SYS_READ_CHAR:
        registers[REG_R0] = (double)sys_read_char(); // Result in R0
        break;
    case OP_SYS_READ_STRING:
        sys_read_string((uint32_t)registers[REG_R0], (uint32_t)registers[REG_R1]); // Assuming addr in R0, max_len in R1
        break;
    case OP_SYS_GET_KEY_PRESS:
        registers[REG_R0] = (double)sys_get_key_press(); // Result in R0
        break;
    case OP_SYS_GET_CPU_VER:
        registers[REG_R0] = sys_get_cpu_ver(); // Version in R0
        break;
    case OP_SYS_WAIT:
        sys_wait((uint32_t)registers[REG_R0]); // Assuming milliseconds in R0
        break;


    case OP_INVALID:
        printf("Invalid Opcode!\n");
        running = false;
        break;
    default:
        printf("Unknown Opcode: %d\n", opcode);
        running = false;
        break;
    }
}

// Run the Virtual Machine
void run_vm() {
    program_counter = 0;
    running = true;
    memset(registers, 0, sizeof(registers));
    registers[REG_SP] = MEMORY_SIZE - 8;
    sys_reset_text_color();
    sys_clear_screen();
    srand(time(NULL));

    while (running) {
        Opcode opcode = decode_opcode();
        execute_instruction(opcode);

        // CPU Speed Simulation (approximate delay)
#ifndef _WIN32
        usleep(DELAY_DURATION_US);
#else
        Sleep(DELAY_DURATION_US / 1000);
#endif

        if (!running) break;
    }
    sys_reset_text_color();
}

// Assembler Functions

// Check if string is a register
bool is_register_str(const char* str) {
    return register_from_string(str) != REG_INVALID;
}

// Check if string represents a memory address (enclosed in brackets)
bool is_memory_address_str(const char* str) {
    return (str[0] == '[' && str[strlen(str) - 1] == ']');
}

// Convert opcode string to Opcode enum
Opcode opcode_from_string(const char* op_str, char* operand1, char* operand2, char* operand3, char* operand4) { // Added operand3
    if (strcmp(op_str, "NOP") == 0) return OP_NOP;
    if (strcmp(op_str, "MOV") == 0) {
        if (operand1 && operand2) {
            if (is_register_str(operand1)) {
                if (is_register_str(operand2)) return OP_MOV_REG_REG;
                else if (is_memory_address_str(operand2)) return OP_MOV_REG_MEM;
                else return OP_MOV_REG_VAL;
            }
            else if (is_memory_address_str(operand1) && is_register_str(operand2)) {
                return OP_MOV_MEM_REG;
            }
        }
    }
    if (strcmp(op_str, "ADD") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_ADD_REG_REG;
            else return OP_ADD_REG_VAL;
        }
    }
    if (strcmp(op_str, "SUB") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_SUB_REG_REG;
            else return OP_SUB_REG_VAL;
        }
    }
    if (strcmp(op_str, "MUL") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_MUL_REG_REG;
            else return OP_MUL_REG_VAL;
        }
    }
    if (strcmp(op_str, "DIV") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_DIV_REG_REG;
            else return OP_DIV_REG_VAL;
        }
    }
    if (strcmp(op_str, "MOD") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_MOD_REG_REG;
            else return OP_MOD_REG_VAL;
        }
    }
    if (strcmp(op_str, "AND") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_AND_REG_REG;
            else return OP_AND_REG_VAL;
        }
    }
    if (strcmp(op_str, "OR") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_OR_REG_REG;
            else return OP_OR_REG_VAL;
        }
    }
    if (strcmp(op_str, "XOR") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_XOR_REG_REG;
            else return OP_XOR_REG_VAL;
        }
    }

    if (strcmp(op_str, "NOT") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_NOT_REG;
    }
    if (strcmp(op_str, "NEG") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_NEG_REG;
    }
    if (strcmp(op_str, "CMP") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_CMP_REG_REG;
            else return OP_CMP_REG_VAL;
        }
    }
    if (strcmp(op_str, "TEST") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_TEST_REG_REG;
            else return OP_TEST_REG_VAL;
        }
    }
    if (strcmp(op_str, "IMUL") == 0) {
        if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_IMUL_REG_REG;
    }
    if (strcmp(op_str, "IDIV") == 0) {
        if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_IDIV_REG_REG;
    }
    if (strcmp(op_str, "MOVZX") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_MOVZX_REG_REG;
            else if (is_memory_address_str(operand2)) return OP_MOVZX_REG_MEM;
        }
    }
    if (strcmp(op_str, "MOVSX") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_MOVSX_REG_REG;
            else if (is_memory_address_str(operand2)) return OP_MOVSX_REG_MEM;
        }
    }

    if (strcmp(op_str, "LEA") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_memory_address_str(operand2) || get_label_address(operand2) != -1) return OP_LEA_REG_MEM;
        }
    }
    if (strcmp(op_str, "JMP") == 0) {
        if (operand1) return OP_JMP;
    }
    if (strcmp(op_str, "JMP_NZ") == 0) {
        if (operand1) return OP_JMP_NZ;
    }
    if (strcmp(op_str, "JMP_Z") == 0) {
        if (operand1) return OP_JMP_Z;
    }
    if (strcmp(op_str, "JMP_S") == 0) {
        if (operand1) return OP_JMP_S;
    }
    if (strcmp(op_str, "JMP_NS") == 0) {
        if (operand1) return OP_JMP_NS;
    }
    if (strcmp(op_str, "JMP_C") == 0) {
        if (operand1) return OP_JMP_C;
    }
    if (strcmp(op_str, "JMP_NC") == 0) {
        if (operand1) return OP_JMP_NC;
    }
    if (strcmp(op_str, "JMP_O") == 0) {
        if (operand1) return OP_JMP_O;
    }
    if (strcmp(op_str, "JMP_NO") == 0) {
        if (operand1) return OP_JMP_NO;
    }

    if (strcmp(op_str, "JMP_GE") == 0) {
        if (operand1) return OP_JMP_GE;
    }
    if (strcmp(op_str, "JMP_LE") == 0) {
        if (operand1) return OP_JMP_LE;
    }
    if (strcmp(op_str, "JMP_G") == 0) {
        if (operand1) return OP_JMP_G;
    }
    if (strcmp(op_str, "JMP_L") == 0) {
        if (operand1) return OP_JMP_L;
    }

    if (strcmp(op_str, "HLT") == 0) return OP_HLT;

    if (strcmp(op_str, "INC") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_INC_REG;
        else if (is_memory_address_str(operand1)) return OP_INC_MEM;
    }
    if (strcmp(op_str, "DEC") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_DEC_REG;
        else if (is_memory_address_str(operand1)) return OP_DEC_MEM;
    }
    if (strcmp(op_str, "SHL") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_SHL_REG_REG;
            else return OP_SHL_REG_VAL;
        }
    }
    if (strcmp(op_str, "SHR") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_SHR_REG_REG;
            else return OP_SHR_REG_VAL;
        }
    }
    if (strcmp(op_str, "SAR") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_SAR_REG_REG;
            else return OP_SAR_REG_VAL;
        }
    }
    if (strcmp(op_str, "ROL") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_ROL_REG_REG;
            else return OP_ROL_REG_VAL;
        }
    }
    if (strcmp(op_str, "ROR") == 0) {
        if (operand1 && operand2 && is_register_str(operand1)) {
            if (is_register_str(operand2)) return OP_ROR_REG_REG;
            else return OP_ROR_REG_VAL;
        }
    }

    if (strcmp(op_str, "RND") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_RND_REG;
    }
    if (strcmp(op_str, "PUSH") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_PUSH_REG;
    }
    if (strcmp(op_str, "POP") == 0) {
        if (operand1 && is_register_str(operand1)) return OP_POP_REG;
    }
    if (strcmp(op_str, "CALL") == 0) {
        if (operand1) return OP_CALL_ADDR;
    }
    if (strcmp(op_str, "RET") == 0) return OP_RET;

    // Math Standard Library Opcodes Parsing
    if (strncmp(op_str, "math.", 5) == 0) {
        char* math_func = op_str + 5;
        if (strcmp(math_func, "add") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_ADD;
        }
        else if (strcmp(math_func, "sub") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_SUB;
        }
        else if (strcmp(math_func, "mul") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MUL;
        }
        else if (strcmp(math_func, "div") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_DIV;
        }
        else if (strcmp(math_func, "mod") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MOD;
        }
        else if (strcmp(math_func, "abs") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_ABS;
        }
        else if (strcmp(math_func, "sin") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_SIN;
        }
        else if (strcmp(math_func, "cos") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_COS;
        }
        else if (strcmp(math_func, "tan") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_TAN;
        }
        else if (strcmp(math_func, "asin") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_ASIN;
        }
        else if (strcmp(math_func, "acos") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_ACOS;
        }
        else if (strcmp(math_func, "atan") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_ATAN;
        }
        else if (strcmp(math_func, "pow") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_POW;
        }
        else if (strcmp(math_func, "sqrt") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_SQRT;
        }
        else if (strcmp(math_func, "log") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_LOG;
        }
        else if (strcmp(math_func, "exp") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_EXP;
        }
        else if (strcmp(math_func, "floor") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_FLOOR;
        }
        else if (strcmp(math_func, "ceil") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_CEIL;
        }
        else if (strcmp(math_func, "round") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_ROUND;
        }
        else if (strcmp(math_func, "min") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MIN;
        }
        else if (strcmp(math_func, "max") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MAX;
        }
        else if (strcmp(math_func, "neg") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_MATH_NEG;
        }
    }

    // String Standard Library Opcodes Parsing
    if (strncmp(op_str, "str.", 4) == 0) {
        char* str_func = op_str + 4;
        if (strcmp(str_func, "len") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) &&
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) // Check for label or [address]
                return OP_STR_LEN_REG_MEM;
        }
        else if (strcmp(str_func, "cpy") == 0) {
            if (operand1 && operand2 &&
                (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && // Check for label or [address]
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) // Check for label or [address]
                return OP_STR_CPY_MEM_MEM;
        }
        else if (strcmp(str_func, "cat") == 0) {
            if (operand1 && operand2 &&
                (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && // Check for label or [address]
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) // Check for label or [address]
                return OP_STR_CAT_MEM_MEM;
        }
        else if (strcmp(str_func, "cmp") == 0) {
            if (operand1 && operand2 && operand3 && is_register_str(operand1) &&
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && // Check for label or [address]
                (is_memory_address_str(operand3) || get_label_address(operand3) != -1)) // Check for label or [address]
                return OP_STR_CMP_REG_MEM_MEM;
        }
        else if (strcmp(str_func, "ncpy") == 0) {
            if (operand1 && operand2 && operand3 &&
                (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && // Check for label or [address]
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && // Check for label or [address]
                is_register_str(operand3))
                return OP_STR_NCPY_MEM_MEM_REG;
        }
        else if (strcmp(str_func, "ncat") == 0) {
            if (operand1 && operand2 && operand3 &&
                (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && // Check for label or [address]
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && // Check for label or [address]
                is_register_str(operand3))
                return OP_STR_NCAT_MEM_MEM_REG;
        }
        else if (strcmp(str_func, "toupper") == 0) {
            if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1))
                return OP_STR_TOUPPER_MEM;
        }
        else if (strcmp(str_func, "tolower") == 0) {
            if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1))
                return OP_STR_TOLOWER_MEM;
        }
        else if (strcmp(str_func, "chr") == 0) {
            if (operand1 && operand2 && operand3 && is_register_str(operand1) &&
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1) &&
                !is_register_str(operand3) && !is_memory_address_str(operand3)) // char_val is immediate
                return OP_STR_CHR_REG_MEM_VAL;
        }
        else if (strcmp(str_func, "str") == 0) {
            if (operand1 && operand2 && operand3 && is_register_str(operand1) &&
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1) &&
                (is_memory_address_str(operand3) || get_label_address(operand3) != -1))
                return OP_STR_STR_REG_MEM_MEM;
        }
    }
    else if (strncmp(op_str, "mem.", 4) == 0) {
        char* mem_func = op_str + 4;
        if (strcmp(mem_func, "cpy") == 0) {
            if (operand1 && operand2 && operand3 &&
                (is_memory_address_str(operand1) || get_label_address(operand1) != -1) &&
                (is_memory_address_str(operand2) || get_label_address(operand2) != -1) &&
                is_register_str(operand3))
                return OP_MEM_CPY_MEM_MEM_REG;
        }
        else if (strcmp(mem_func, "set") == 0) {
            if (operand1 && operand2 && operand3 &&
                (is_memory_address_str(operand1) || get_label_address(operand1) != -1) &&
                is_register_str(operand2) &&
                !is_register_str(operand3) && !is_memory_address_str(operand3)) // Value is immediate
                return OP_MEM_SET_MEM_REG_VAL;
        }
        else if (strcmp(mem_func, "clear") == 0) {
            if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1)) {
                return OP_MEM_FREE_MEM;
            }
        }
    }
    // System Standard Library Opcodes Parsing
    else if (strncmp(op_str, "sys.", 4) == 0) {
        char* sys_func = op_str + 4;
        if (strcmp(sys_func, "print_char") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_PRINT_CHAR;
        }
        else if (strcmp(sys_func, "clear_screen") == 0) {
            return OP_SYS_CLEAR_SCREEN;
        }
        else if (strcmp(sys_func, "print_string") == 0) {
            if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1 || is_register_str(operand1))) return OP_SYS_PRINT_STRING;
        }
        else if (strcmp(sys_func, "newline") == 0) {
            return OP_SYS_PRINT_NEWLINE;
        }
        else if (strcmp(sys_func, "set_cursor_pos") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_SYS_SET_CURSOR_POS;
        }
        else if (strcmp(sys_func, "get_cursor_pos") == 0) {
            return OP_SYS_GET_CURSOR_POS;
        }
        else if (strcmp(sys_func, "set_text_color") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_SET_TEXT_COLOR;
        }
        else if (strcmp(sys_func, "reset_text_color") == 0) {
            return OP_SYS_RESET_TEXT_COLOR;
        }
        else if (strcmp(sys_func, "print_number_dec") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_PRINT_NUMBER_DEC;
        }
        else if (strcmp(sys_func, "print_number_hex") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_PRINT_NUMBER_HEX;
        }
        else if (strcmp(sys_func, "number_to_string") == 0) {
            if (operand1 && operand2 && operand3 && is_register_str(operand1) && is_register_str(operand2) && is_register_str(operand3)) return OP_SYS_NUMBER_TO_STRING;
        }
        else if (strcmp(sys_func, "read_char") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_READ_CHAR; // Though read_char doesn't take an arg, we expect a reg for consistency
        }
        else if (strcmp(sys_func, "read_string") == 0) {
            if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_SYS_READ_STRING;
        }
        else if (strcmp(sys_func, "get_key_press") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_GET_KEY_PRESS; // Similar to read_char
        }
        else if (strcmp(sys_func, "cpu_ver") == 0) {
            if (operand1 && is_register_str(operand1)) return OP_SYS_GET_CPU_VER;
        }
        else if (strcmp(sys_func, "wait") == 0) {
            if (operand1) return OP_SYS_WAIT;
        }
    }


    return OP_INVALID;
}

// Convert register string to RegisterIndex enum
RegisterIndex register_from_string(const char* reg_str) {
    if (strcmp(reg_str, "SP") == 0) return REG_SP;
    if (strcmp(reg_str, "ZF") == 0) return REG_ZF;
    if (strcmp(reg_str, "SF") == 0) return REG_SF;
    if (strcmp(reg_str, "CF") == 0) return REG_CF;
    if (strcmp(reg_str, "OF") == 0) return REG_OF;
    if (strlen(reg_str) >= 2 && reg_str[0] == 'R') {
        int reg_num = atoi(reg_str + 1);
        if (reg_num >= 0 && reg_num < NUM_GENERAL_REGISTERS) {
            return (RegisterIndex)reg_num;
        }
    }
    else if (strlen(reg_str) >= 2 && reg_str[0] == 'F') {
        int reg_num = atoi(reg_str + 1);
        if (reg_num >= 0 && reg_num < NUM_F_REGISTERS) {
            return (RegisterIndex)(REG_F0 + reg_num);
        }
    }
    return REG_INVALID;
}

// Get macro value by name
const char* get_macro_value(const char* macro_name) {
    for (int i = 0; i < macro_count; i++) {
        if (strcmp(macros[i].name, macro_name) == 0) {
            return macros[i].value_str;
        }
    }
    return NULL;
}

// Get label address by name
uint32_t get_label_address(const char* label_name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, label_name) == 0) {
            return labels[i].address;
        }
    }
    for (int i = 0; i < string_count; i++) {
        if (strcmp(strings[i].name, label_name) == 0) {
            return strings[i].address;
        }
    }
    for (int i = 0; i < buffer_count; i++) {
        if (strcmp(buffers[i].name, label_name) == 0) {
            return buffers[i].address;
        }
    }
    return -1; // Label not found
}

// Parse value string to double (supports hex, decimal, float, char literals, macros, labels)
double parse_value_double(const char* value_str) {
    if (strcmp(value_str, "M_PI") == 0) {
        return M_PI;
    }
    if (strcmp(value_str, "M_E") == 0) {
        return M_E;
    }
    const char* macro_value_str = get_macro_value(value_str);
    if (macro_value_str != NULL) {
        value_str = macro_value_str;
    }

    if (value_str[0] == '\'' && value_str[strlen(value_str) - 1] == '\'') {
        if (strlen(value_str) == 3) {
            return (double)value_str[1];
        }
        else {
            fprintf(stderr, "Error: Invalid character literal '%s'\n", value_str);
            return 0.0;
        }
    }
    else if (strncmp(value_str, "0x", 2) == 0) {
        return (double)strtol(value_str + 2, NULL, 16);
    }
    else if (strncmp(value_str, "0b", 2) == 0) {
        return (double)strtol(value_str + 2, NULL, 2);
    }
    else if (isalpha(value_str[0]) || value_str[0] == '_') {
        uint32_t label_addr = get_label_address(value_str);
        if (label_addr != -1) {
            return (double)label_addr;
        }
        else {
            return strtod(value_str, NULL);
        }
    }
    else {
        return strtod(value_str, NULL);
    }
}

// Parse address string
uint32_t parse_address(const char* addr_str) {
    char temp_addr_str[64];
    strncpy(temp_addr_str, addr_str, sizeof(temp_addr_str) - 1);
    temp_addr_str[sizeof(temp_addr_str) - 1] = '\0';

    if (temp_addr_str[0] == '[') {
        if (temp_addr_str[strlen(temp_addr_str) - 1] == ']') {
            temp_addr_str[strlen(temp_addr_str) - 1] = '\0';
            return (uint32_t)parse_value_double(temp_addr_str + 1);
        }
        else {
            fprintf(stderr, "Error: Unmatched '[' in address '%s'\n", addr_str);
            return 0;
        }
    }
    return (uint32_t)parse_value_double(temp_addr_str);
}

// Preprocessor State for Conditional Assembly
typedef enum {
    PREPROCESSOR_STATE_NORMAL,
    PREPROCESSOR_STATE_IFDEF_FALSE
} PreprocessorState;

// Assemble program from assembly file to ROM file
int assemble_program(const char* asm_filename, const char* rom_filename) {
    FILE* asm_file = fopen(asm_filename, "r");
    if (!asm_file) {
        perror("Error opening assembly file");
        return -1;
    }

    FILE* rom_file = fopen(rom_filename, "wb");
    if (!rom_file) {
        perror("Error opening ROM file for writing");
        fclose(asm_file);
        return -1;
    }

    memset(memory, 0, MEMORY_SIZE); // Clear memory
    program_counter = 0;          // Reset program counter
    macro_count = 0;              // Reset macro count
    label_count = 0;              // Reset label count
    string_count = 0;             // Reset string count
    buffer_count = 0;             // Reset buffer count
    data_section_start = 0;       // Reset data section start
    uint32_t rom_offset = 0;      // ROM offset, defaults to 0

    char line[256];
    int line_number = 1;
    PreprocessorState preprocessor_state[MAX_PREPROCESSOR_DEPTH]; // Preprocessor state stack
    int preprocessor_depth = 0;                                 // Preprocessor nesting depth
    preprocessor_state[0] = PREPROCESSOR_STATE_NORMAL;          // Initial state is normal

    // --- Pass 1: Scan for labels, macros, strings, buffers, calculate program size ---
    rewind(asm_file); // Reset file pointer to beginning
    program_counter = 0;
    while (fgets(line, sizeof(line), asm_file)) {
        char* line_ptr = line;
        char* token = strtok(line_ptr, " ,\t\n"); // Tokenize line
        if (!token || token[0] == ';') {         // Skip empty lines and comments
            line_number++;
            continue;
        }

        // Preprocessor directives handling
        if (preprocessor_state[preprocessor_depth] == PREPROCESSOR_STATE_IFDEF_FALSE) {
            if (strcmp(token, "#endif") == 0) {
                if (preprocessor_depth > 0) {
                    preprocessor_depth--; // Exit #ifdef block
                }
                else {
                    fprintf(stderr, "Error: Unmatched #endif directive on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#ifdef") == 0 || strcmp(token, "#ifndef") == 0) {
                if (preprocessor_depth < MAX_PREPROCESSOR_DEPTH - 1) {
                    preprocessor_depth++;
                    preprocessor_state[preprocessor_depth] = PREPROCESSOR_STATE_IFDEF_FALSE; // Nested IFDEF is also false initially
                }
                else {
                    fprintf(stderr, "Error: Max preprocessor nesting depth reached on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#else") == 0) {
                if (preprocessor_depth > 0) {
                    preprocessor_state[preprocessor_depth] = (preprocessor_state[preprocessor_depth] == PREPROCESSOR_STATE_NORMAL) ? PREPROCESSOR_STATE_IFDEF_FALSE : PREPROCESSOR_STATE_NORMAL;
                }
                else {
                    fprintf(stderr, "Error: #else directive without matching #ifdef or #ifndef on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            line_number++;
            continue; // Skip assembly in false #ifdef block
        }

        // Handle offset directive
        if (strcmp(token, "#offset") == 0) {
            char* offset_str = strtok(NULL, " ,\t\n");
            if (offset_str) {
                rom_offset = parse_address(offset_str);
                if (rom_offset > MEMORY_SIZE) {
                    fprintf(stderr, "Error: Offset too large on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
                if (rom_offset % 1 != 0) {
                    fprintf(stderr, "Error: Offset must be byte aligned on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }

            }
            else {
                fprintf(stderr, "Error: Missing offset value in #offset directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            line_number++;
            continue;
        }
        // Handle string definition directive
        else if (strcmp(token, ".STRING") == 0) {
            char* string_name = strtok(NULL, " ,\t\n");
            char* string_value_token = strtok(NULL, "\n"); // Get the rest of the line as string value

            if (!string_name) {
                fprintf(stderr, "Error: Missing string name in .STRING directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            if (!string_value_token) {
                fprintf(stderr, "Error: Missing string value in .STRING directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }

            const char* macro_value = get_macro_value(string_value_token);
            const char* string_value_with_quotes;
            if (macro_value != NULL) {
                string_value_with_quotes = macro_value; // Use macro value directly
            }
            else {
                string_value_with_quotes = string_value_token; // Assume it's a string literal
            }

            if (string_count < 1024) {
                strncpy(strings[string_count].name, string_name, sizeof(strings[string_count].name) - 1);
                strings[string_count].name[sizeof(strings[string_count].name) - 1] = '\0';

                char* start_quote = strchr(string_value_with_quotes, '\'');
                char* end_quote = strrchr(string_value_with_quotes, '\'');
                if (start_quote && end_quote && start_quote != end_quote) {
                    size_t len = end_quote - start_quote - 1;
                    if (len < sizeof(strings[string_count].value)) {
                        strncpy(strings[string_count].value, start_quote + 1, len);
                        strings[string_count].value[len] = '\0';
                    }
                    else {
                        fprintf(stderr, "Error: String value too long on line %d.\n", line_number);
                        fclose(asm_file);
                        fclose(rom_file);
                        return -1;
                    }
                }
                else {
                    // If no quotes found but we are using macro, it might be ok if macro value is already a string
                    if (macro_value == NULL) { // Error only if it's not a macro and no quotes
                        fprintf(stderr, "Error: Invalid string syntax on line %d. String must be enclosed in single quotes.\n", line_number);
                        fclose(asm_file);
                        fclose(rom_file);
                        return -1;
                    }
                    else {
                        strncpy(strings[string_count].value, string_value_with_quotes, sizeof(strings[string_count].value) - 1);
                        strings[string_count].value[sizeof(strings[string_count].value) - 1] = '\0';
                    }
                }
                string_count++;
            }
            else {
                fprintf(stderr, "Error: String limit reached on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            line_number++;
            continue;
        }
        // Handle buffer definition directive
        else if (strcmp(token, ".BUFFER") == 0) {
            char* buffer_name = strtok(NULL, " ,\t\n");
            char* buffer_size_str = strtok(NULL, " ,\t\n");

            if (!buffer_name) {
                fprintf(stderr, "Error: Missing buffer name in .BUFFER directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            if (!buffer_size_str) {
                fprintf(stderr, "Error: Missing buffer size in .BUFFER directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }

            uint32_t buffer_size = parse_address(buffer_size_str);
            if (buffer_size == 0 || buffer_size > MEMORY_SIZE) {
                fprintf(stderr, "Error: Invalid buffer size '%u' on line %d.\n", buffer_size, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }

            if (buffer_count < 1024) {
                strncpy(buffers[buffer_count].name, buffer_name, sizeof(buffers[buffer_count].name) - 1);
                buffers[buffer_count].name[sizeof(buffers[buffer_count].name) - 1] = '\0';
                buffers[buffer_count].size = buffer_size;
                buffer_count++;
                program_counter += buffer_size; // Reserve space in memory for buffer
            }
            else {
                fprintf(stderr, "Error: Buffer limit reached on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            line_number++;
            continue;
        }
        // Handle preprocessor directives
        else if (token[0] == '#') {
            if (strcmp(token, "#define") == 0) {
                char* macro_name = strtok(NULL, " ,\t\n");
                char* macro_value = strtok(NULL, " ,\t\n");
                if (macro_name && macro_value) {
                    if (macro_count < 1024) {
                        strncpy(macros[macro_count].name, macro_name, sizeof(macros[macro_count].name) - 1);
                        macros[macro_count].name[sizeof(macros[macro_count].name) - 1] = '\0';
                        strncpy(macros[macro_count].value_str, macro_value, sizeof(macros[macro_count].value_str) - 1);
                        macros[macro_count].value_str[sizeof(macros[macro_count].value_str) - 1] = '\0';
                        macro_count++;
                    }
                    else {
                        fprintf(stderr, "Error: Macro limit reached on line %d.\n", line_number);
                        fclose(asm_file);
                        fclose(rom_file);
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "Error: Invalid #define syntax on line %d. Expected #define MACRO_NAME VALUE\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#ifdef") == 0) {
                if (preprocessor_depth < MAX_PREPROCESSOR_DEPTH - 1) {
                    char* macro_to_check = strtok(NULL, " ,\t\n");
                    if (macro_to_check) {
                        if (get_macro_value(macro_to_check) != NULL) {
                            preprocessor_depth++;
                            preprocessor_state[preprocessor_depth] = PREPROCESSOR_STATE_NORMAL;
                        }
                        else {
                            preprocessor_depth++;
                            preprocessor_state[preprocessor_depth] = PREPROCESSOR_STATE_IFDEF_FALSE;
                        }
                    }
                    else {
                        fprintf(stderr, "Error: Missing macro name in #ifdef directive on line %d.\n", line_number);
                        fclose(asm_file);
                        fclose(rom_file);
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "Error: Max preprocessor nesting depth reached on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#ifndef") == 0) {
                if (preprocessor_depth < MAX_PREPROCESSOR_DEPTH - 1) {
                    char* macro_to_check = strtok(NULL, " ,\t\n");
                    if (macro_to_check) {
                        if (get_macro_value(macro_to_check) == NULL) {
                            preprocessor_depth++;
                            preprocessor_state[preprocessor_depth] = PREPROCESSOR_STATE_NORMAL;
                        }
                        else {
                            preprocessor_depth++;
                            preprocessor_state[preprocessor_depth] = PREPROCESSOR_STATE_IFDEF_FALSE;
                        }
                    }
                    else {
                        fprintf(stderr, "Error: Missing macro name in #ifndef directive on line %d.\n", line_number);
                        fclose(asm_file);
                        fclose(rom_file);
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "Error: Max preprocessor nesting depth reached on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#else") == 0) {
                if (preprocessor_depth > 0) {
                    preprocessor_state[preprocessor_depth] = (preprocessor_state[preprocessor_depth] == PREPROCESSOR_STATE_NORMAL) ? PREPROCESSOR_STATE_IFDEF_FALSE : PREPROCESSOR_STATE_NORMAL;
                }
                else {
                    fprintf(stderr, "Error: #else directive without matching #ifdef or #ifndef on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#endif") == 0) {
                if (preprocessor_depth > 0) {
                    preprocessor_depth--;
                }
                else {
                    fprintf(stderr, "Error: Unmatched #endif directive on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
                }
            }
            else if (strcmp(token, "#error") == 0) {
                char* error_message = strtok(NULL, "\n");
                if (error_message) {
                    fprintf(stderr, "Error on line %d: %s\n", line_number, error_message);
                }
                else {
                    fprintf(stderr, "Error on line %d: #error directive without message.\n", line_number);
                }
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            else if (strcmp(token, "#warning") == 0) {
                char* warning_message = strtok(NULL, "\n");
                if (warning_message) {
                    fprintf(stderr, "Warning on line %d: %s\n", line_number, warning_message);
                }
                else {
                    fprintf(stderr, "Warning on line %d: #warning directive without message.\n", line_number);
                }
            }
            else {
                fprintf(stderr, "Error: Unknown preprocessor directive '%s' on line %d.\n", token, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            line_number++;
            continue;
        }
        // Handle labels
        else if (strchr(token, ':') != NULL && strlen(token) > 1) {
            token[strlen(token) - 1] = '\0'; // Remove colon
            if (label_count < 1024) {
                strncpy(labels[label_count].name, token, sizeof(labels[label_count].name) - 1);
                labels[label_count].name[sizeof(labels[label_count].name) - 1] = '\0';
                labels[label_count].address = program_counter;
                label_count++;
            }
            else {
                fprintf(stderr, "Error: Label limit reached on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            line_number++;
            continue;
        }

        char* operand1_str = strtok(NULL, " ,\t\n");
        char* operand2_str = strtok(NULL, " ,\t\n");
        char* operand3_str = strtok(NULL, " ,\t\n");
        char* operand4_str = strtok(NULL, " ,\t\n");
        Opcode opcode = opcode_from_string(token, operand1_str, operand2_str, operand3_str, operand4_str); // Decode opcode

        if (opcode == OP_INVALID) {
            fprintf(stderr, "Error: Invalid opcode '%s' on line %d.\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }

        memory[program_counter++] = (uint8_t)opcode; // Write opcode to memory

        // Increment program counter based on instruction length (updated for string opcodes)
        switch (opcode) {
        case OP_MOV_REG_VAL:
        case OP_ADD_REG_VAL:
        case OP_SUB_REG_VAL:
        case OP_MUL_REG_VAL:
        case OP_DIV_REG_VAL:
        case OP_MOD_REG_VAL:
        case OP_CMP_REG_VAL:
        case OP_TEST_REG_VAL:
            program_counter += 9; break; // Opcode (1) + Reg (1) + Double (8)
        case OP_MOV_REG_REG:
        case OP_ADD_REG_REG:
        case OP_SUB_REG_REG:
        case OP_MUL_REG_REG:
        case OP_DIV_REG_REG:
        case OP_MOD_REG_REG:
        case OP_AND_REG_REG:
        case OP_OR_REG_REG:
        case OP_XOR_REG_REG:
        case OP_CMP_REG_REG:
        case OP_TEST_REG_REG:
        case OP_IMUL_REG_REG:
        case OP_IDIV_REG_REG:
        case OP_MOVZX_REG_REG:
        case OP_MOVSX_REG_REG:
        case OP_SHL_REG_REG:
        case OP_SHR_REG_REG:
        case OP_SAR_REG_REG:
        case OP_ROL_REG_REG:
        case OP_ROR_REG_REG:
        case OP_MATH_ADD:
        case OP_MATH_SUB:
        case OP_MATH_MUL:
        case OP_MATH_DIV:
        case OP_MATH_MOD:
        case OP_MATH_POW:
        case OP_MATH_MIN:
        case OP_MATH_MAX:
            program_counter += 2; break; // Opcode (1) + 2 Regs (2)
        case OP_MOV_REG_MEM:
        case OP_MOV_MEM_REG:
        case OP_LEA_REG_MEM:
        case OP_MOVZX_REG_MEM:
        case OP_MOVSX_REG_MEM:
        case OP_STR_LEN_REG_MEM:
            program_counter += 5; break; // Opcode (1) + Reg (1) + Address (4)
        case OP_NOT_REG:
        case OP_NEG_REG:
        case OP_INC_REG:
        case OP_DEC_REG:
        case OP_RND_REG:
        case OP_PUSH_REG:
        case OP_POP_REG:
        case OP_MATH_ABS:
        case OP_MATH_SIN:
        case OP_MATH_COS:
        case OP_MATH_TAN:
        case OP_MATH_ASIN:
        case OP_MATH_ACOS:
        case OP_MATH_ATAN:
        case OP_MATH_SQRT:
        case OP_MATH_LOG:
        case OP_MATH_EXP:
        case OP_MATH_FLOOR:
        case OP_MATH_CEIL:
        case OP_MATH_ROUND:
        case OP_MATH_NEG:
        case OP_SHL_REG_VAL:
        case OP_SHR_REG_VAL:
        case OP_SAR_REG_VAL:
        case OP_ROL_REG_VAL:
        case OP_ROR_REG_VAL:
            program_counter += 2; break; // Opcode (1) + Reg (1) or Value (1)
        case OP_JMP:
        case OP_JMP_NZ:
        case OP_JMP_Z:
        case OP_JMP_S:
        case OP_JMP_NS:
        case OP_JMP_C:
        case OP_JMP_NC:
        case OP_JMP_O:
        case OP_JMP_NO:
        case OP_JMP_GE:
        case OP_JMP_LE:
        case OP_JMP_G:
        case OP_JMP_L:
        case OP_CALL_ADDR:
            program_counter += 4; break; // Opcode (1) + Address (4)
        case OP_INC_MEM:
        case OP_DEC_MEM:
        case OP_STR_CPY_MEM_MEM:
        case OP_STR_CAT_MEM_MEM:
            program_counter += 8; break; // Opcode (1) + 2 Addresses (8)
        case OP_STR_CMP_REG_MEM_MEM:
            program_counter += 9; break; // Opcode (1) + Reg (1) + 2 Addresses (8)
        case OP_STR_NCPY_MEM_MEM_REG:
        case OP_STR_NCAT_MEM_MEM_REG:
            program_counter += 9; break; // Opcode (1) + Reg (1) + 2 Addresses (8)
        case OP_MEM_CPY_MEM_MEM_REG:
            program_counter += 9; break; // Opcode (1) + 2 Addresses (8) + Reg (1)
        case OP_MEM_SET_MEM_REG_VAL:
            program_counter += 10; break; // Opcode (1) + Address (4) + Reg (1) + Value (4) - assuming value is uint32 immediate
        case OP_MEM_FREE_MEM:
            program_counter += 5; break; // Opcode (1) + Address (4)
        case OP_STR_TOUPPER_MEM:
        case OP_STR_TOLOWER_MEM:
            program_counter += 5; break; // Opcode (1) + Address (4)
        case OP_STR_CHR_REG_MEM_VAL:
            program_counter += 10; break; // Opcode (1) + Reg (1) + Address (4) + Value (4)
        case OP_STR_STR_REG_MEM_MEM:
            program_counter += 9; break; // Opcode (1) + Reg (1) + 2 Addresses (8)
            // System Library Opcodes have no operands after opcode byte itself in Pass 1
        case OP_SYS_PRINT_CHAR:
        case OP_SYS_CLEAR_SCREEN:
        case OP_SYS_PRINT_STRING:
        case OP_SYS_PRINT_NEWLINE:
        case OP_SYS_SET_CURSOR_POS:
        case OP_SYS_GET_CURSOR_POS:
        case OP_SYS_SET_TEXT_COLOR:
        case OP_SYS_RESET_TEXT_COLOR:
        case OP_SYS_PRINT_NUMBER_DEC:
        case OP_SYS_PRINT_NUMBER_HEX:
        case OP_SYS_NUMBER_TO_STRING:
        case OP_SYS_READ_CHAR:
        case OP_SYS_READ_STRING:
        case OP_SYS_GET_KEY_PRESS:
        case OP_SYS_GET_CPU_VER:
        case OP_SYS_WAIT:
        case OP_NOP:
        case OP_HLT:
        case OP_RET:
            break; // Opcode (1)
        default:
            fprintf(stderr, "Assembler Error (Pass 1): Unhandled opcode size calculation for '%s' on line %d.\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }
        line_number++;
    }
    if (preprocessor_depth != 0) {
        fprintf(stderr, "Error: Unclosed #ifdef or #ifndef block.\n");
        fclose(asm_file);
        fclose(rom_file);
        return -1;
    }

    data_section_start = program_counter; // Data section starts after code

    // --- Pass 2: Assemble instructions and data ---
    rewind(asm_file); // Reset file pointer again
    program_counter = 0;
    line_number = 1;
    uint32_t data_pointer = data_section_start; // Data pointer starts at data section start
    preprocessor_depth = 0;                     // Reset preprocessor depth
    preprocessor_state[0] = PREPROCESSOR_STATE_NORMAL; // Reset preprocessor state

    // Write ROM offset padding if any
    for (uint32_t i = 0; i < rom_offset; ++i) {
        fputc(0x00, rom_file);
    }

    uint32_t rom_code_start = rom_offset; // ROM code starts after offset padding

    while (fgets(line, sizeof(line), asm_file)) {
        char* line_ptr = line;
        char* token = strtok(line_ptr, " ,\t\n");
        if (!token || token[0] == ';') {
            line_number++;
            continue;
        }

        // Preprocessor directives handling
        if (preprocessor_state[preprocessor_depth] == PREPROCESSOR_STATE_IFDEF_FALSE) {
            if (strcmp(token, "#endif") == 0) {
                if (preprocessor_depth > 0) {
                    preprocessor_depth--;
                }
            }
            else if (strcmp(token, "#ifdef") == 0 || strcmp(token, "#ifndef") == 0) {
                if (preprocessor_depth < MAX_PREPROCESSOR_DEPTH - 1) {
                    preprocessor_depth++;
                    preprocessor_state[preprocessor_depth] = PREPROCESSOR_STATE_IFDEF_FALSE;
                }
            }
            else if (strcmp(token, "#else") == 0) {
                if (preprocessor_depth > 0) {
                    preprocessor_state[preprocessor_depth] = (preprocessor_state[preprocessor_depth] == PREPROCESSOR_STATE_NORMAL) ? PREPROCESSOR_STATE_IFDEF_FALSE : PREPROCESSOR_STATE_NORMAL;
                }
            }
            line_number++;
            continue;
        }


        if (strcmp(token, "#offset") == 0) {
            line_number++;
            continue; // Ignore offset directive in pass 2, already processed in pass 1
        }
        // Handle string data placement
        if (strcmp(token, ".STRING") == 0) {
            char* string_name = strtok(NULL, " ,\t\n");
            char* string_value_token = strtok(NULL, "\n");
            const char* macro_value = get_macro_value(string_value_token);
            const char* string_value_with_quotes = macro_value != NULL ? macro_value : string_value_token;

            for (int i = 0; i < string_count; i++) {
                if (strcmp(strings[i].name, string_name) == 0) {
                    strings[i].address = data_pointer; // Assign data address
                    strcpy((char*)&memory[data_pointer], strings[i].value); // Copy string value to memory
                    data_pointer += strlen(strings[i].value) + 1;         // Increment data pointer
                    break;
                }
            }
            line_number++;
            continue;
        }
        // Handle buffer data placement
        if (strcmp(token, ".BUFFER") == 0) {
            char* buffer_name = strtok(NULL, " ,\t\n");

            for (int i = 0; i < buffer_count; i++) {
                if (strcmp(buffers[i].name, buffer_name) == 0) {
                    buffers[i].address = data_pointer; // Assign data address
                    data_pointer += buffers[i].size;    // Increment data pointer by buffer size
                    break;
                }
            }
            line_number++;
            continue;
        }
        // Skip preprocessor directives and labels in pass 2, already handled in pass 1
        if (token[0] == '#' || (strchr(token, ':') != NULL && strlen(token) > 1)) {
            line_number++;
            continue;
        }

        char* reg1_str = strtok(NULL, " ,\t\n");
        char* reg2_str = strtok(NULL, " ,\t\n");
        char* reg3_str = strtok(NULL, " ,\t\n");
        char* reg4_str = strtok(NULL, " ,\t\n");
        Opcode opcode = opcode_from_string(token, reg1_str, reg2_str, reg3_str, reg4_str); // Decode opcode again

        if (opcode == OP_INVALID) {
            fprintf(stderr, "Error: Invalid opcode '%s' on line %d (Pass 2).\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }

        memory[program_counter++] = (uint8_t)opcode; // Write opcode to memory again

        // Write instruction operands to memory based on opcode
        switch (opcode) {
        case OP_MOV_REG_VAL:
        case OP_ADD_REG_VAL:
        case OP_SUB_REG_VAL:
        case OP_MUL_REG_VAL:
        case OP_DIV_REG_VAL:
        case OP_MOD_REG_VAL:
        case OP_CMP_REG_VAL:
        case OP_TEST_REG_VAL:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in MOV instruction on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            double value = parse_value_double(reg2_str); // Parse value operand as double
            memory[program_counter++] = (uint8_t)reg;
            *(double*)&memory[program_counter] = value; // Write double value to memory
            program_counter += 8;
            break;
        }
        case OP_MOV_REG_REG:
        case OP_ADD_REG_REG:
        case OP_SUB_REG_REG:
        case OP_MUL_REG_REG:
        case OP_DIV_REG_REG:
        case OP_MOD_REG_REG:
        case OP_AND_REG_REG:
        case OP_OR_REG_REG:
        case OP_XOR_REG_REG:
        case OP_CMP_REG_REG:
        case OP_TEST_REG_REG:
        case OP_IMUL_REG_REG:
        case OP_IDIV_REG_REG:
        case OP_MOVZX_REG_REG:
        case OP_MOVSX_REG_REG:
        case OP_SHL_REG_REG:
        case OP_SHR_REG_REG:
        case OP_SAR_REG_REG:
        case OP_ROL_REG_REG:
        case OP_ROR_REG_REG:
        case OP_MATH_ADD:
        case OP_MATH_SUB:
        case OP_MATH_MUL:
        case OP_MATH_DIV:
        case OP_MATH_MOD:
        case OP_MATH_POW:
        case OP_MATH_MIN:
        case OP_MATH_MAX:
        {
            RegisterIndex reg1 = register_from_string(reg1_str);
            RegisterIndex reg2 = register_from_string(reg2_str);
            if (reg1 == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' as destination in instruction on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            if (reg2 == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' as source in instruction on line %d.\n", reg2_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            memory[program_counter++] = (uint8_t)reg1;
            memory[program_counter++] = (uint8_t)reg2;
            break;
        }
        case OP_LEA_REG_MEM:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            char* addr_str = reg2_str;
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in LEA instruction on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t address = parse_address(addr_str); // Parse memory address
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4; // Address is 4 bytes
            break;
        }
        case OP_MOV_REG_MEM:
        case OP_MOVZX_REG_MEM:
        case OP_MOVSX_REG_MEM:
        case OP_STR_LEN_REG_MEM:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            char* addr_str = reg2_str;
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in MOVRM instruction on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t address = parse_address(addr_str); // Parse memory address
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4; // Address is still 4 bytes
            break;
        }
        case OP_MOV_MEM_REG: {
            char* addr_str = reg1_str;
            RegisterIndex reg = register_from_string(reg2_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in MOVMR instruction on line %d.\n", reg2_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t address = parse_address(addr_str); // Parse memory address
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4; // Address is still 4 bytes
            memory[program_counter++] = (uint8_t)reg;
            break;
        }
        case OP_NOT_REG:
        case OP_NEG_REG:
        case OP_INC_REG:
        case OP_DEC_REG:
        case OP_RND_REG:
        case OP_PUSH_REG:
        case OP_POP_REG:
        case OP_MATH_ABS:
        case OP_MATH_SIN:
        case OP_MATH_COS:
        case OP_MATH_TAN:
        case OP_MATH_ASIN:
        case OP_MATH_ACOS:
        case OP_MATH_ATAN:
        case OP_MATH_SQRT:
        case OP_MATH_LOG:
        case OP_MATH_EXP:
        case OP_MATH_FLOOR:
        case OP_MATH_CEIL:
        case OP_MATH_ROUND:
        case OP_MATH_NEG:
        case OP_SHL_REG_VAL:
        case OP_SHR_REG_VAL:
        case OP_SAR_REG_VAL:
        case OP_ROL_REG_VAL:
        case OP_ROR_REG_VAL:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in instruction on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            memory[program_counter++] = (uint8_t)reg;
            break;
        }
        case OP_JMP:
        case OP_JMP_NZ:
        case OP_JMP_Z:
        case OP_JMP_S:
        case OP_JMP_NS:
        case OP_JMP_C:
        case OP_JMP_NC:
        case OP_JMP_O:
        case OP_JMP_NO:
        case OP_JMP_GE:
        case OP_JMP_LE:
        case OP_JMP_G:
        case OP_JMP_L:
        case OP_CALL_ADDR: {
            uint32_t address = parse_address(reg1_str); // Parse jump address as integer address
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            break;
        }
        case OP_INC_MEM:
        case OP_DEC_MEM:
        {
            uint32_t address = parse_address(reg1_str);
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            break;
        }
        case OP_STR_CPY_MEM_MEM:
        case OP_STR_CAT_MEM_MEM: {
            uint32_t dest_addr = parse_address(reg1_str);
            uint32_t src_addr = parse_address(reg2_str);
            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = src_addr;
            program_counter += 4;
            break;
        }
        case OP_STR_CMP_REG_MEM_MEM: {
            RegisterIndex reg = register_from_string(reg1_str);
            uint32_t addr1 = parse_address(reg2_str);
            uint32_t addr2 = parse_address(reg3_str);
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = addr1;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = addr2;
            program_counter += 4;
            break;
        }
        case OP_STR_NCPY_MEM_MEM_REG:
        case OP_STR_NCAT_MEM_MEM_REG: {
            uint32_t dest_addr = parse_address(reg1_str);
            uint32_t src_addr = parse_address(reg2_str);
            RegisterIndex reg = register_from_string(reg3_str);
            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = src_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg;
            break;
        }
        case OP_MEM_CPY_MEM_MEM_REG: {
            uint32_t dest_addr = parse_address(reg1_str);
            uint32_t src_addr = parse_address(reg2_str);
            RegisterIndex reg_count = register_from_string(reg3_str);
            if (reg_count == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' for count in MEMCPY on line %d.\n", reg3_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = src_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg_count;
            break;
        }
        case OP_MEM_SET_MEM_REG_VAL: {
            uint32_t dest_addr = parse_address(reg1_str);
            RegisterIndex reg_value = register_from_string(reg2_str);
            uint32_t count_val = parse_address(reg3_str); // Parse count as immediate value

            if (reg_value == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' for value in MEMSET on line %d.\n", reg2_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }

            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg_value;
            *(uint32_t*)&memory[program_counter] = count_val; // Write count value
            program_counter += 4;
            break;
        }
        case OP_MEM_FREE_MEM: {
            uint32_t address = parse_address(reg1_str);
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            break;
        }
        case OP_STR_TOUPPER_MEM:
        case OP_STR_TOLOWER_MEM:
        {
            uint32_t address = parse_address(reg1_str);
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            break;
        }
        case OP_STR_CHR_REG_MEM_VAL:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            uint32_t address = parse_address(reg2_str);
            uint32_t char_val = parse_address(reg3_str); // Parse char_val as address to handle immediate values
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in STRCHR on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = char_val; // Write char value
            program_counter += 4;
            break;
        }
        case OP_STR_STR_REG_MEM_MEM:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            uint32_t haystack_addr = parse_address(reg2_str);
            uint32_t needle_addr = parse_address(reg3_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in STRSTR on line %d.\n", reg1_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = haystack_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = needle_addr;
            program_counter += 4;
            break;
        }
        // System Library Opcodes - operands are registers, already available in registers array
        case OP_SYS_PRINT_CHAR: // Argument in R0
        case OP_SYS_CLEAR_SCREEN:
        case OP_SYS_PRINT_STRING: // Address in R0
        case OP_SYS_PRINT_NEWLINE:
        case OP_SYS_SET_CURSOR_POS: // X in R0, Y in R1
        case OP_SYS_GET_CURSOR_POS: // Results in R0, R1 (X, Y)
        case OP_SYS_SET_TEXT_COLOR: // Color code in R0
        case OP_SYS_RESET_TEXT_COLOR:
        case OP_SYS_PRINT_NUMBER_DEC: // Number in R0
        case OP_SYS_PRINT_NUMBER_HEX: // Number in R0
        case OP_SYS_NUMBER_TO_STRING: // Num in R0, addr in R1, size in R2
        case OP_SYS_READ_CHAR: // Result in R0
        case OP_SYS_READ_STRING: // Addr in R0, max_len in R1
        case OP_SYS_GET_KEY_PRESS: // Result in R0
        case OP_SYS_GET_CPU_VER: // Version in R0
        case OP_SYS_WAIT: // Milliseconds in R0
        case OP_RET:
        case OP_NOP:
        case OP_HLT:
            break;
        default:
            fprintf(stderr, "Assembler Error: Unhandled opcode in assembler switch on line %d (Pass 2).\n", line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }
        line_number++;
    }

    fwrite(memory + rom_code_start, 1, data_pointer - rom_code_start, rom_file); // Write assembled code and data to ROM file

    fclose(asm_file);
    fclose(rom_file);
    printf("Successfully assembled '%s' to '%s'\n", asm_filename, rom_filename);
    return 0;
}

// Load ROM file into memory
int load_rom(const char* rom_filename) {
    FILE* rom_file = fopen(rom_filename, "rb");
    if (!rom_file) {
        perror("Error opening ROM file for reading");
        return -1;
    }

    memset(memory, 0, MEMORY_SIZE); // Clear memory before loading ROM
    fseek(rom_file, 0, SEEK_END);
    long rom_size = ftell(rom_file); // Get ROM file size
    rewind(rom_file);              // Reset file pointer to beginning

    if (rom_size > MEMORY_SIZE) {
        fprintf(stderr, "Error: ROM file is too large to load into memory.\n");
        fclose(rom_file);
        return -1;
    }

    size_t bytes_read = fread(memory, 1, rom_size, rom_file); // Read ROM into memory
    fclose(rom_file);
    printf("Loaded %zu bytes from '%s'\n", bytes_read, rom_filename);
    return 0;
}

int main(int argc, char* argv[]) {
    char choice;
    char filename[256];

    while (1) {
        printf("\nChoose action:\n");
        printf("1. Assemble .asm to .rom\n");
        printf("2. Run .rom\n");
        printf("3. Exit\n");
        printf("Enter choice (1-3): ");
        scanf(" %c", &choice);

        switch (choice) {
        case '1':
            printf("Enter assembly filename (.asm): ");
            scanf("%255s", filename);
            if (assemble_program(filename, "output.rom") == 0) {
                printf("Assembly successful. ROM file 'output.rom' created.\n");
            }
            else {
                fprintf(stderr, "Assembly failed.\n");
            }
            break;
        case '2':
            if (load_rom("output.rom") == 0) {
                printf("Running 'output.rom'...\n");
                run_vm();
                printf("\n\nVM execution finished.\n");
            }
            else {
                fprintf(stderr, "Failed to load ROM.\n");
            }
            break;
        case '3':
            printf("Exiting.\n");
            return 0;
        default:
            printf("Invalid choice. Please enter 1, 2, or 3.\n");
        }
    }

    return 0;
}
