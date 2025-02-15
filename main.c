#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif
#include <time.h>

#define MEMORY_SIZE (640 * 1024)
#define NUM_REGISTERS 32
#define MAX_MACROS 100
#define MAX_LABELS 100
#define MAX_STRINGS 100
#define CPU_VER 1

typedef enum {
    OP_NOP,
    OP_MOV_REG_VAL,
    OP_MOV_REG_REG,
    OP_MOV_REG_MEM,
    OP_MOV_MEM_REG,
    OP_ADD_REG_REG,
    OP_SUB_REG_REG,
    OP_MUL_REG_REG,
    OP_DIV_REG_REG,
    OP_MOD_REG_REG,
    OP_AND_REG_REG,
    OP_OR_REG_REG,
    OP_XOR_REG_REG,
    OP_NOT_REG,
    OP_CMP_REG_REG,
    OP_CMP_REG_VAL,
    OP_INT,
    OP_JMP,
    OP_JMP_NZ,
    OP_JMP_Z,
    OP_JMP_S,
    OP_JMP_NS,
    OP_JMP_C,
    OP_JMP_NC,
    OP_JMP_O,
    OP_JMP_NO,
    OP_HLT,
    OP_INC_REG,
    OP_DEC_REG,
    OP_SHL_REG_REG,
    OP_SHL_REG_VAL,
    OP_SHR_REG_REG,
    OP_SHR_REG_VAL,
    OP_SAR_REG_REG,
    OP_SAR_REG_VAL,
    OP_ROL_REG_REG,
    OP_ROL_REG_VAL,
    OP_ROR_REG_REG,
    OP_ROR_REG_VAL,
    OP_RND_REG,
    OP_INVALID
} Opcode;

typedef enum {
    REG_R0, REG_R1, REG_R2, REG_R3, REG_R4, REG_R5, REG_R6, REG_R7,
    REG_R8, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15,
    REG_R16, REG_R17, REG_R18, REG_R19, REG_R20, REG_R21, REG_R22, REG_R23,
    REG_R24, REG_R25, REG_R26, REG_R27, REG_R28, REG_R29, REG_R30, REG_R31,
    REG_INVALID
} RegisterIndex;

typedef enum {
    BIOS_PRINT_CHAR = 0x01,
    BIOS_CLEAR_SCREEN = 0x02,
    BIOS_WAIT = 0x03,
    BIOS_PRINT_STRING = 0x04,
    BIOS_PRINT_NEWLINE = 0x05,
    BIOS_READ_CHAR = 0x06,
    BIOS_READ_STRING = 0x07,
    BIOS_PRINT_NUMBER_DEC = 0x08,
    BIOS_PRINT_NUMBER_HEX = 0x09,
    BIOS_SET_CURSOR_POS = 0x0A,
    BIOS_GET_CURSOR_POS = 0x0B,
    BIOS_SET_TEXT_COLOR = 0x0C,
    BIOS_RESET_TEXT_COLOR = 0x0D,
    BIOS_GET_CPU_VER = 0x01,
    BIOS_INVALID_FUNCTION = 0xFF // Example for invalid function
} BIOSFunction;

typedef enum {
    BIOS_INT_VIDEO = 0x01,
    BIOS_INT_KEYBOARD = 0x02,
    BIOS_INT_SYSTEM = 0x03,
    BIOS_INVALID_INT = 0xFF // Example for invalid interrupt
} BIOSInterrupt;


typedef struct {
    char name[32];
    char value_str[32];
} MacroDefinition;

typedef struct {
    char name[32];
    uint32_t address;
} LabelDefinition;

typedef struct {
    char name[32];
    uint32_t address;
    char value[256];
} StringDefinition;

uint8_t memory[MEMORY_SIZE];
uint32_t registers[NUM_REGISTERS];
uint32_t program_counter = 0;
bool running = true;
bool zero_flag = false;
bool sign_flag = false;
bool carry_flag = false;
bool overflow_flag = false;
MacroDefinition macros[MAX_MACROS];
int macro_count = 0;
LabelDefinition labels[MAX_LABELS];
int label_count = 0;
StringDefinition strings[MAX_STRINGS];
int string_count = 0;
uint32_t data_section_start = 0;

int cursor_x = 0;
int cursor_y = 0;
int text_color = 7;

#ifndef _WIN32
struct termios original_termios;

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}
#endif

char bios_read_char() {
#ifdef _WIN32
    return _getch();
#else
    enable_raw_mode();
    char c = getchar();
    disable_raw_mode();
    return c;
#endif
}

void bios_print_char(char character) {
    printf("%c", character);
}

void bios_print_newline() {
    bios_print_char('\n');
    cursor_y++;
    cursor_x = 0;
}

void bios_read_string(uint32_t address, uint32_t max_len) {
    if (address >= MEMORY_SIZE) {
        printf("Error: String address out of memory bounds in READ_STRING.\n");
        return;
    }
    char* str_ptr = (char*)&memory[address];
    uint32_t i = 0;
    char c;
    while (i < max_len - 1) {
        c = bios_read_char();
        if (c == '\r' || c == '\n') {
            str_ptr[i] = '\0';
            bios_print_newline();
            break;
        }
        else if (c == 127) {
            if (i > 0) {
                i--;
                str_ptr[i] = '\0';
                printf("\b \b");
            }
        }
        else if (c >= 32 && c <= 126) {
            str_ptr[i++] = c;
            bios_print_char(c);
        }
        else if (c == 27) {
            continue;
        }
    }
    if (i == max_len - 1) {
        str_ptr[i] = '\0';
    }
}


void bios_print_number_dec(uint32_t number) {
    printf("%u", number);
}

void bios_print_number_hex(uint32_t number) {
    printf("0x%X", number);
}

void bios_set_cursor_pos(uint32_t x, uint32_t y) {
    cursor_x = x;
    cursor_y = y;
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, pos);
#else
    printf("\033[%d;%dH", y + 1, x + 1);
#endif
}

void bios_get_cursor_pos(uint32_t* x, uint32_t* y) {
    *x = cursor_x;
    *y = cursor_y;
}

void bios_set_text_color(uint32_t color_code) {
    text_color = color_code;
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color_code);
#else
    switch (color_code) {
    case 0: printf("\033[30m"); break;
    case 1: printf("\033[34m"); break;
    case 2: printf("\033[32m"); break;
    case 3: printf("\033[36m"); break;
    case 4: printf("\033[31m"); break;
    case 5: printf("\033[35m"); break;
    case 6: printf("\033[33m"); break;
    case 7: printf("\033[37m"); break;
    default: printf("\033[37m"); break;
    }
#endif
}

void bios_reset_text_color() {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    printf("\033[0m");
#endif
    text_color = 7;
}

void bios_print_string(uint32_t address) {
    if (address >= MEMORY_SIZE) {
        printf("Error: String address out of memory bounds.\n");
        return;
    }
    char* str = (char*)&memory[address];
    while (*str != '\0') {
        bios_print_char(*str);
        str++;
        if ((uint32_t)(str - (char*)memory) >= MEMORY_SIZE) {
            printf("Error: String exceeds memory bounds.\n");
            return;
        }
    }
}

void bios_clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    cursor_x = 0;
    cursor_y = 0;
}

void bios_wait(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void bios_interrupt(uint8_t interrupt_number) {
    BIOSInterrupt bios_int = (BIOSInterrupt)interrupt_number;
    BIOSFunction function = (BIOSFunction)registers[REG_R0];

    switch (bios_int) {
    case BIOS_INT_VIDEO:
        switch (function) {
        case BIOS_PRINT_CHAR:
            bios_print_char((char)registers[REG_R1]);
            cursor_x++;
            break;
        case BIOS_CLEAR_SCREEN:
            bios_clear_screen();
            break;
        case BIOS_WAIT:
            bios_wait(registers[REG_R1]);
            break;
        case BIOS_PRINT_STRING:
            bios_print_string(registers[REG_R1]);
            break;
        case BIOS_PRINT_NEWLINE:
            bios_print_newline();
            break;
        default:
            printf("BIOS Interrupt 0x%02X (Video) - Unknown function: 0x%02X\n", bios_int, function);
            break;
        }
        break;
    case BIOS_INT_KEYBOARD:
        switch (function) {
        case BIOS_READ_CHAR:
            registers[REG_R1] = (uint32_t)bios_read_char();
            break;
        case BIOS_READ_STRING:
            bios_read_string(registers[REG_R1], registers[REG_R2]);
            break;
        case BIOS_PRINT_NUMBER_DEC:
            bios_print_number_dec(registers[REG_R1]);
            break;
        case BIOS_PRINT_NUMBER_HEX:
            bios_print_number_hex(registers[REG_R1]);
            break;
        case BIOS_SET_CURSOR_POS:
            bios_set_cursor_pos(registers[REG_R1], registers[REG_R2]);
            break;
        case BIOS_GET_CURSOR_POS:
            bios_get_cursor_pos(registers[REG_R1], registers[REG_R2]);
            break;
        case BIOS_SET_TEXT_COLOR:
            bios_set_text_color(registers[REG_R1]);
            break;
        case BIOS_RESET_TEXT_COLOR:
            bios_reset_text_color();
            break;
        default:
            printf("BIOS Interrupt 0x%02X (Keyboard/IO) - Unknown function: 0x%02X\n", bios_int, function);
            break;
        }
        break;
    case BIOS_INT_SYSTEM:
        switch (function) {
        case BIOS_GET_CPU_VER:
            registers[REG_R1] = CPU_VER;
            break;
        default:
            printf("BIOS Interrupt 0x%02X (System) - Unknown function: 0x%02X\n", bios_int, function);
            break;
        }
        break;
    default:
        printf("Unknown BIOS Interrupt: 0x%02X\n", interrupt_number);
        break;
    }
}

Opcode decode_opcode() {
    if (program_counter >= MEMORY_SIZE) return OP_INVALID;
    return (Opcode)memory[program_counter++];
}

RegisterIndex decode_register() {
    if (program_counter >= MEMORY_SIZE) return REG_INVALID;
    uint8_t reg_index = memory[program_counter++];
    if (reg_index >= NUM_REGISTERS) return REG_INVALID;
    return (RegisterIndex)reg_index;
}

uint32_t decode_value() {
    if (program_counter + 4 > MEMORY_SIZE) return 0;
    uint32_t value = *(uint32_t*)&memory[program_counter];
    program_counter += 4;
    return value;
}

uint32_t decode_address() {
    return decode_value();
}

void set_zero_flag(uint32_t result) {
    zero_flag = (result == 0);
}

void set_sign_flag(uint32_t result) {
    sign_flag = ((int32_t)result < 0);
}

void set_carry_flag_add(uint32_t a, uint32_t b) {
    carry_flag = ((uint64_t)a + b > 0xFFFFFFFF);
}

void set_carry_flag_sub(uint32_t a, uint32_t b) {
    carry_flag = (a < b);
}

void set_overflow_flag_add(uint32_t a, uint32_t b) {
    int32_t signed_a = (int32_t)a;
    int32_t signed_b = (int32_t)b;
    int32_t signed_result = signed_a + signed_b;
    overflow_flag = ((signed_a >= 0 && signed_b >= 0 && signed_result < 0) || (signed_a < 0 && signed_b < 0 && signed_result >= 0));
}

void set_overflow_flag_sub(uint32_t a, uint32_t b) {
    int32_t signed_a = (int32_t)a;
    int32_t signed_b = (int32_t)b;
    int32_t signed_result = signed_a - signed_b;
    overflow_flag = ((signed_a >= 0 && signed_b < 0 && signed_result < 0) || (signed_a < 0 && signed_b >= 0 && signed_result >= 0));
}

void execute_instruction(Opcode opcode) {
    RegisterIndex reg1, reg2;
    uint32_t value, address;

    switch (opcode) {
    case OP_NOP:
        break;
    case OP_MOV_REG_VAL:
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) registers[reg1] = value;
        break;
    case OP_MOV_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) registers[reg1] = registers[reg2];
        break;
    case OP_MOV_REG_MEM:
        reg1 = decode_register();
        address = decode_address();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE - 4) registers[reg1] = *(uint32_t*)&memory[address];
        break;
    case OP_MOV_MEM_REG:
        address = decode_address();
        reg1 = decode_register();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE - 4) *(uint32_t*)&memory[address] = registers[reg1];
        break;
    case OP_ADD_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = registers[reg1];
            uint32_t val2 = registers[reg2];
            set_carry_flag_add(val1, val2);
            set_overflow_flag_add(val1, val2);
            registers[reg1] += registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_SUB_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = registers[reg1];
            uint32_t val2 = registers[reg2];
            set_carry_flag_sub(val1, val2);
            set_overflow_flag_sub(val1, val2);
            registers[reg1] -= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_MUL_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] *= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_DIV_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (registers[reg2] != 0) {
                registers[reg1] /= registers[reg2];
                set_zero_flag(registers[reg1]);
                set_sign_flag(registers[reg1]);
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
            if (registers[reg2] != 0) {
                registers[reg1] %= registers[reg2];
                set_zero_flag(registers[reg1]);
                set_sign_flag(registers[reg1]);
            }
            else {
                printf("Modulo by zero!\n");
                running = false;
            }
        }
        break;
    case OP_AND_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] &= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_OR_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] |= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_XOR_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] ^= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_NOT_REG:
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = ~registers[reg1];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_CMP_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t val1 = registers[reg1];
            uint32_t val2 = registers[reg2];
            set_carry_flag_sub(val1, val2);
            set_overflow_flag_sub(val1, val2);
            set_zero_flag(val1 - val2);
            set_sign_flag(val1 - val2);
        }
        break;
    case OP_CMP_REG_VAL:
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = registers[reg1];
            uint32_t val2 = value;
            set_carry_flag_sub(val1, val2);
            set_overflow_flag_sub(val1, val2);
            set_zero_flag(val1 - val2);
            set_sign_flag(val1 - val2);
        }
        break;
    case OP_INT:
        value = decode_value();
        bios_interrupt((uint8_t)value);
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
    case OP_HLT:
        running = false;
        break;
    case OP_INC_REG:
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = registers[reg1];
            uint32_t val2 = 1;
            set_carry_flag_add(val1, val2);
            set_overflow_flag_add(val1, val2);
            registers[reg1]++;
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_DEC_REG:
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = registers[reg1];
            uint32_t val2 = 1;
            set_carry_flag_sub(val1, val2);
            set_overflow_flag_sub(val1, val2);
            registers[reg1]--;
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    case OP_SHL_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] <<= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    case OP_SHL_REG_VAL:
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) {
            registers[reg1] <<= value;
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    case OP_SHR_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            registers[reg1] >>= registers[reg2];
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    case OP_SHR_REG_VAL:
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) {
            registers[reg1] >>= value;
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    case OP_SAR_REG_REG:
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            int32_t signed_val = (int32_t)registers[reg1];
            signed_val >>= registers[reg2];
            registers[reg1] = (uint32_t)signed_val;
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    case OP_SAR_REG_VAL:
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) {
            int32_t signed_val = (int32_t)registers[reg1];
            signed_val >>= value;
            registers[reg1] = (uint32_t)signed_val;
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    case OP_ROL_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t shift_amount = registers[reg2] % 32;
            registers[reg1] = (registers[reg1] << shift_amount) | (registers[reg1] >> (32 - shift_amount));
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    }
    case OP_ROL_REG_VAL: {
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) {
            uint32_t shift_amount = value % 32;
            registers[reg1] = (registers[reg1] << shift_amount) | (registers[reg1] >> (32 - shift_amount));
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    }
    case OP_ROR_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            uint32_t shift_amount = registers[reg2] % 32;
            registers[reg1] = (registers[reg1] >> shift_amount) | (registers[reg1] << (32 - shift_amount));
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    }
    case OP_ROR_REG_VAL: {
        reg1 = decode_register();
        value = decode_value();
        if (reg1 != REG_INVALID) {
            uint32_t shift_amount = value % 32;
            registers[reg1] = (registers[reg1] >> shift_amount) | (registers[reg1] << (32 - shift_amount));
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
            carry_flag = false;
            overflow_flag = false;
        }
        break;
    }
    case OP_RND_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = rand();
            set_zero_flag(registers[reg1]);
            set_sign_flag(registers[reg1]);
        }
        break;
    }
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

void run_vm() {
    program_counter = 0;
    running = true;
    memset(registers, 0, sizeof(registers));
    bios_reset_text_color();
    bios_clear_screen();
    srand(time(NULL));
    while (running) {
        Opcode opcode = decode_opcode();
        execute_instruction(opcode);
        if (!running) break;
    }
    bios_reset_text_color();
}

Opcode opcode_from_string(const char* op_str) {
    if (strcmp(op_str, "NOP") == 0) return OP_NOP;
    if (strcmp(op_str, "MOV") == 0) return OP_MOV_REG_VAL;
    if (strcmp(op_str, "MOVRR") == 0) return OP_MOV_REG_REG;
    if (strcmp(op_str, "MOVRM") == 0) return OP_MOV_REG_MEM;
    if (strcmp(op_str, "MOVMR") == 0) return OP_MOV_MEM_REG;
    if (strcmp(op_str, "ADD") == 0) return OP_ADD_REG_REG;
    if (strcmp(op_str, "SUB") == 0) return OP_SUB_REG_REG;
    if (strcmp(op_str, "MUL") == 0) return OP_MUL_REG_REG;
    if (strcmp(op_str, "DIV") == 0) return OP_DIV_REG_REG;
    if (strcmp(op_str, "MOD") == 0) return OP_MOD_REG_REG;
    if (strcmp(op_str, "AND") == 0) return OP_AND_REG_REG;
    if (strcmp(op_str, "OR") == 0) return OP_OR_REG_REG;
    if (strcmp(op_str, "XOR") == 0) return OP_XOR_REG_REG;
    if (strcmp(op_str, "NOT") == 0) return OP_NOT_REG;
    if (strcmp(op_str, "CMP") == 0) return OP_CMP_REG_REG;
    if (strcmp(op_str, "CMPV") == 0) return OP_CMP_REG_VAL;
    if (strcmp(op_str, "INT") == 0) return OP_INT;
    if (strcmp(op_str, "JMP") == 0) return OP_JMP;
    if (strcmp(op_str, "JMP_NZ") == 0) return OP_JMP_NZ;
    if (strcmp(op_str, "JMP_Z") == 0) return OP_JMP_Z;
    if (strcmp(op_str, "JMP_S") == 0) return OP_JMP_S;
    if (strcmp(op_str, "JMP_NS") == 0) return OP_JMP_NS;
    if (strcmp(op_str, "JMP_C") == 0) return OP_JMP_C;
    if (strcmp(op_str, "JMP_NC") == 0) return OP_JMP_NC;
    if (strcmp(op_str, "JMP_O") == 0) return OP_JMP_O;
    if (strcmp(op_str, "JMP_NO") == 0) return OP_JMP_NO;
    if (strcmp(op_str, "HLT") == 0) return OP_HLT;
    if (strcmp(op_str, "INC") == 0) return OP_INC_REG;
    if (strcmp(op_str, "DEC") == 0) return OP_DEC_REG;
    if (strcmp(op_str, "SHL") == 0) return OP_SHL_REG_REG;
    if (strcmp(op_str, "SHLV") == 0) return OP_SHL_REG_VAL;
    if (strcmp(op_str, "SHR") == 0) return OP_SHR_REG_REG;
    if (strcmp(op_str, "SHRV") == 0) return OP_SHR_REG_VAL;
    if (strcmp(op_str, "SAR") == 0) return OP_SAR_REG_REG;
    if (strcmp(op_str, "SARV") == 0) return OP_SAR_REG_VAL;
    if (strcmp(op_str, "ROL") == 0) return OP_ROL_REG_REG;
    if (strcmp(op_str, "ROLV") == 0) return OP_ROL_REG_VAL;
    if (strcmp(op_str, "ROR") == 0) return OP_ROR_REG_REG;
    if (strcmp(op_str, "RORV") == 0) return OP_ROR_REG_VAL;
    if (strcmp(op_str, "RND") == 0) return OP_RND_REG;
    return OP_INVALID;
}

RegisterIndex register_from_string(const char* reg_str) {
    if (strlen(reg_str) >= 2 && reg_str[0] == 'R') {
        int reg_num = atoi(reg_str + 1);
        if (reg_num >= 0 && reg_num < NUM_REGISTERS) {
            return (RegisterIndex)reg_num;
        }
    }
    return REG_INVALID;
}

const char* get_macro_value(const char* macro_name) {
    for (int i = 0; i < macro_count; i++) {
        if (strcmp(macros[i].name, macro_name) == 0) {
            return macros[i].value_str;
        }
    }
    return NULL;
}

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
    return -1;
}

uint32_t parse_value(const char* value_str) {
    const char* macro_value_str = get_macro_value(value_str);
    if (macro_value_str != NULL) {
        value_str = macro_value_str;
    }

    if (value_str[0] == '\'' && value_str[strlen(value_str) - 1] == '\'') {
        if (strlen(value_str) == 3) {
            return (uint32_t)value_str[1];
        }
        else {
            fprintf(stderr, "Error: Invalid character literal '%s'\n", value_str);
            return 0;
        }
    }
    else if (strncmp(value_str, "0x", 2) == 0) {
        return strtol(value_str + 2, NULL, 16);
    }
    else if (isalpha(value_str[0]) || value_str[0] == '_') {
        uint32_t label_addr = get_label_address(value_str);
        if (label_addr != -1) {
            return label_addr;
        }
        else {
            return atoi(value_str);
        }
    }
    else {
        return atoi(value_str);
    }
}

uint32_t parse_address(const char* addr_str) {
    char temp_addr_str[64];
    strncpy(temp_addr_str, addr_str, sizeof(temp_addr_str) - 1);
    temp_addr_str[sizeof(temp_addr_str) - 1] = '\0';

    if (temp_addr_str[0] == '[') {
        if (temp_addr_str[strlen(temp_addr_str) - 1] == ']') {
            temp_addr_str[strlen(temp_addr_str) - 1] = '\0';
            return parse_value(temp_addr_str + 1);
        }
        else {
            fprintf(stderr, "Error: Unmatched '[' in address '%s'\n", addr_str);
            return 0;
        }
    }
    return parse_value(temp_addr_str);
}

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

    memset(memory, 0, MEMORY_SIZE);
    program_counter = 0;
    macro_count = 0;
    label_count = 0;
    string_count = 0;
    data_section_start = 0;

    char line[256];
    int line_number = 1;

    rewind(asm_file);
    program_counter = 0;
    while (fgets(line, sizeof(line), asm_file)) {
        char* line_ptr = line;
        char* token = strtok(line_ptr, " ,\t\n");
        if (!token || token[0] == ';') {
            line_number++;
            continue;
        }

        if (strcmp(token, ".STRING") == 0) {
            char* string_name = strtok(NULL, " ,\t\n");
            char* string_value_with_quotes = strtok(NULL, "\n");

            if (!string_name) {
                fprintf(stderr, "Error: Missing string name in .STRING directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            if (!string_value_with_quotes) {
                fprintf(stderr, "Error: Missing string value in .STRING directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }

            if (string_count < MAX_STRINGS) {
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
                    fprintf(stderr, "Error: Invalid string syntax on line %d. String must be enclosed in single quotes.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    return -1;
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
        else if (token[0] == '#') {
            if (strcmp(token, "#define") == 0) {
                char* macro_name = strtok(NULL, " ,\t\n");
                char* macro_value = strtok(NULL, " ,\t\n");
                if (macro_name && macro_value) {
                    if (macro_count < MAX_MACROS) {
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
            else {
                fprintf(stderr, "Error: Unknown preprocessor directive '%s' on line %d.\n", token, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            line_number++;
            continue;
        }
        else if (strchr(token, ':') != NULL && strlen(token) > 1) {
            token[strlen(token) - 1] = '\0';
            if (label_count < MAX_LABELS) {
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

        Opcode opcode = opcode_from_string(token);
        if (opcode == OP_INVALID) {
            fprintf(stderr, "Error: Invalid opcode '%s' on line %d.\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }

        memory[program_counter++] = (uint8_t)opcode;

        switch (opcode) {
        case OP_MOV_REG_VAL: program_counter += 5; break;
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
        case OP_SHL_REG_REG:
        case OP_SHR_REG_REG:
        case OP_SAR_REG_REG:
        case OP_ROL_REG_REG:
        case OP_ROR_REG_REG:
            program_counter += 2; break;
        case OP_CMP_REG_VAL:
        case OP_MOV_REG_MEM:
        case OP_MOV_MEM_REG:
        case OP_SHL_REG_VAL:
        case OP_SHR_REG_VAL:
        case OP_SAR_REG_VAL:
        case OP_ROL_REG_VAL:
        case OP_ROR_REG_VAL:
            program_counter += 5; break;
        case OP_NOT_REG:
        case OP_INC_REG:
        case OP_DEC_REG:
        case OP_RND_REG: program_counter += 1; break;
        case OP_INT: program_counter += 4; break;
        case OP_JMP:
        case OP_JMP_NZ:
        case OP_JMP_Z:
        case OP_JMP_S:
        case OP_JMP_NS:
        case OP_JMP_C:
        case OP_JMP_NC:
        case OP_JMP_O:
        case OP_JMP_NO: program_counter += 4; break;
        case OP_NOP:
        case OP_HLT: break;
        default:
            fprintf(stderr, "Assembler Error (Pass 1): Unhandled opcode size calculation for '%s' on line %d.\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }
        line_number++;
    }
    data_section_start = program_counter;

    rewind(asm_file);
    program_counter = 0;
    line_number = 1;
    uint32_t data_pointer = data_section_start;

    while (fgets(line, sizeof(line), asm_file)) {
        char* line_ptr = line;
        char* token = strtok(line_ptr, " ,\t\n");
        if (!token || token[0] == ';') {
            line_number++;
            continue;
        }

        if (strcmp(token, ".STRING") == 0) {
            char* string_name = strtok(NULL, " ,\t\n");
            for (int i = 0; i < string_count; i++) {
                if (strcmp(strings[i].name, string_name) == 0) {
                    strings[i].address = data_pointer;
                    strcpy((char*)&memory[data_pointer], strings[i].value);
                    data_pointer += strlen(strings[i].value) + 1;
                    break;
                }
            }
            line_number++;
            continue;
        }
        if (token[0] == '#' || (strchr(token, ':') != NULL && strlen(token) > 1)) {
            line_number++;
            continue;
        }

        Opcode opcode = opcode_from_string(token);
        if (opcode == OP_INVALID) {
            fprintf(stderr, "Error: Invalid opcode '%s' on line %d (Pass 2).\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            return -1;
        }

        memory[program_counter++] = (uint8_t)opcode;

        switch (opcode) {
        case OP_MOV_REG_VAL: {
            char* reg_str = strtok(NULL, " ,\t\n");
            char* value_str = strtok(NULL, " ,\t\n");
            RegisterIndex reg = register_from_string(reg_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in MOV instruction on line %d.\n", reg_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t value = parse_value(value_str);
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = value;
            program_counter += 4;
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
        case OP_SHL_REG_REG:
        case OP_SHR_REG_REG:
        case OP_SAR_REG_REG:
        case OP_ROL_REG_REG:
        case OP_ROR_REG_REG:
        {
            char* reg1_str = strtok(NULL, " ,\t\n");
            char* reg2_str = strtok(NULL, " ,\t\n");
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
        case OP_MOV_REG_MEM: {
            char* reg_str = strtok(NULL, " ,\t\n");
            char* addr_str = strtok(NULL, " ,\t\n");
            RegisterIndex reg = register_from_string(reg_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in MOVRM instruction on line %d.\n", reg_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t address = parse_address(addr_str);
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            break;
        }
        case OP_MOV_MEM_REG: {
            char* addr_str = strtok(NULL, " ,\t\n");
            char* reg_str = strtok(NULL, " ,\t\n");
            uint32_t address = parse_address(addr_str);
            RegisterIndex reg = register_from_string(reg_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in MOVMR instruction on line %d.\n", reg_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg;
            break;
        }
        case OP_NOT_REG:
        case OP_INC_REG:
        case OP_DEC_REG:
        case OP_RND_REG: {
            char* reg_str = strtok(NULL, " ,\t\n");
            RegisterIndex reg = register_from_string(reg_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in instruction on line %d.\n", reg_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            memory[program_counter++] = (uint8_t)reg;
            break;
        }
        case OP_CMP_REG_VAL: {
            char* reg_str = strtok(NULL, " ,\t\n");
            char* value_str = strtok(NULL, " ,\t\n");
            RegisterIndex reg = register_from_string(reg_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in CMP instruction on line %d.\n", reg_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t value = parse_value(value_str);
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = value;
            program_counter += 4;
            break;
        }
        case OP_INT: {
            char* value_str = strtok(NULL, " ,\t\n");
            uint32_t value = parse_value(value_str);
            *(uint32_t*)&memory[program_counter] = value;
            program_counter += 4;
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
        case OP_JMP_NO: {
            char* addr_str = strtok(NULL, " ,\t\n");
            uint32_t address = parse_address(addr_str);
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            break;
        }
        case OP_SHL_REG_VAL:
        case OP_SHR_REG_VAL:
        case OP_SAR_REG_VAL:
        case OP_ROL_REG_VAL:
        case OP_ROR_REG_VAL:
        {
            char* reg_str = strtok(NULL, " ,\t\n");
            char* value_str = strtok(NULL, " ,\t\n");
            RegisterIndex reg = register_from_string(reg_str);
            if (reg == REG_INVALID) {
                fprintf(stderr, "Error: Invalid register '%s' in shift/rotate instruction on line %d.\n", reg_str, line_number);
                fclose(asm_file);
                fclose(rom_file);
                return -1;
            }
            uint32_t value = parse_value(value_str);
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = value;
            program_counter += 4;
            break;
        }
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

    fwrite(memory, 1, data_pointer, rom_file);

    fclose(asm_file);
    fclose(rom_file);
    printf("Successfully assembled '%s' to '%s'\n", asm_filename, rom_filename);
    return 0;
}

int load_rom(const char* rom_filename) {
    FILE* rom_file = fopen(rom_filename, "rb");
    if (!rom_file) {
        perror("Error opening ROM file for reading");
        return -1;
    }

    memset(memory, 0, MEMORY_SIZE);
    size_t bytes_read = fread(memory, 1, MEMORY_SIZE, rom_file);
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
