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
#include <limits.h>
#endif
#include <time.h>

#define MEMORY_SIZE (16384 * 1024) // 16MB Memory
#define NUM_GENERAL_REGISTERS 32
#define CPU_VER 4
#define MAX_PREPROCESSOR_DEPTH 10

#define CLOCK_SPEED_MHZ 4.77
#define TARGET_INSTRUCTIONS_PER_SECOND 100000
#define DELAY_DURATION_US (1000000 / TARGET_INSTRUCTIONS_PER_SECOND)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#define MAX_MACROS 65536
#define MAX_LABELS 65536
#define MAX_STRINGS 65536
#define MAX_BUFFERS 65536

typedef enum {
    OP_NOP = 0,
    OP_MOV_REG_VAL, OP_MOV_REG_REG, OP_MOV_REG_MEM, OP_MOV_MEM_REG,
    OP_ADD_REG_REG, OP_ADD_REG_VAL,
    OP_SUB_REG_REG, OP_SUB_REG_VAL,
    OP_MUL_REG_REG, OP_MUL_REG_VAL,
    OP_DIV_REG_REG, OP_DIV_REG_VAL,
    OP_MOD_REG_REG, OP_MOD_REG_VAL,
    OP_AND_REG_REG, OP_AND_REG_VAL,
    OP_OR_REG_REG, OP_OR_REG_VAL,
    OP_XOR_REG_REG, OP_XOR_REG_VAL,
    OP_NOT_REG, OP_NEG_REG,
    OP_CMP_REG_REG, OP_CMP_REG_VAL,
    OP_TEST_REG_REG, OP_TEST_REG_VAL,
    OP_IMUL_REG_REG, OP_IDIV_REG_REG,
    OP_MOVZX_REG_REG, OP_MOVZX_REG_MEM,
    OP_MOVSX_REG_REG, OP_MOVSX_REG_MEM,
    OP_LEA_REG_MEM,
    OP_JMP, OP_JMP_NZ, OP_JMP_Z, OP_JMP_S, OP_JMP_NS,
    OP_JMP_C, OP_JMP_NC, OP_JMP_O, OP_JMP_NO, OP_JMP_GE, OP_JMP_LE, OP_JMP_G, OP_JMP_L,
    OP_HLT,
    OP_INC_REG, OP_DEC_REG, OP_INC_MEM, OP_DEC_MEM,
    OP_SHL_REG_REG, OP_SHL_REG_VAL,
    OP_SHR_REG_REG, OP_SHR_REG_VAL,
    OP_SAR_REG_REG, OP_SAR_REG_VAL,
    OP_ROL_REG_REG, OP_ROL_REG_VAL,
    OP_ROR_REG_REG, OP_ROR_REG_VAL,
    OP_RND_REG,
    OP_PUSH_REG, OP_POP_REG,
    OP_CALL_ADDR, OP_RET,
    OP_XCHG_REG_REG, OP_BSWAP_REG, OP_SETZ_REG, OP_SETNZ_REG,

    // Math Standard Library
    OP_MATH_ADD, OP_MATH_SUB, OP_MATH_MUL, OP_MATH_DIV, OP_MATH_MOD,
    OP_MATH_ABS, OP_MATH_SIN, OP_MATH_COS, OP_MATH_TAN, OP_MATH_ASIN,
    OP_MATH_ACOS, OP_MATH_ATAN, OP_MATH_POW, OP_MATH_SQRT, OP_MATH_LOG,
    OP_MATH_EXP, OP_MATH_FLOOR, OP_MATH_CEIL, OP_MATH_ROUND, OP_MATH_MIN, OP_MATH_MAX, OP_MATH_NEG,
    OP_MATH_ATAN2, OP_MATH_LOG10, OP_MATH_CLAMP, OP_MATH_LERP,

    // String Standard Library
    OP_STR_LEN_REG_MEM, OP_STR_CPY_MEM_MEM, OP_STR_CAT_MEM_MEM, OP_STR_CMP_REG_MEM_MEM,
    OP_STR_NCPY_MEM_MEM_REG, OP_STR_NCAT_MEM_MEM_REG, OP_STR_TOUPPER_MEM, OP_STR_TOLOWER_MEM,
    OP_STR_CHR_REG_MEM_VAL, OP_STR_STR_REG_MEM_MEM, OP_STR_ATOI_REG_MEM, OP_STR_ITOA_MEM_REG_REG, OP_STR_SUBSTR_MEM_MEM_REG_REG,
    OP_STR_FMT_MEM_MEM_REG_REG,

    // Memory Standard Library
    OP_MEM_CPY_MEM_MEM_REG, OP_MEM_SET_MEM_REG_VAL, OP_MEM_SET_MEM_REG_REG, OP_MEM_FREE_MEM,

    // System Standard Library
    OP_SYS_PRINT_CHAR, OP_SYS_CLEAR_SCREEN, OP_SYS_PRINT_STRING, OP_SYS_PRINT_NEWLINE,
    OP_SYS_SET_CURSOR_POS, OP_SYS_GET_CURSOR_POS, OP_SYS_SET_TEXT_COLOR, OP_SYS_RESET_TEXT_COLOR,
    OP_SYS_PRINT_NUMBER_DEC, OP_SYS_PRINT_NUMBER_HEX, OP_SYS_NUMBER_TO_STRING, OP_SYS_READ_CHAR,
    OP_SYS_READ_STRING, OP_SYS_GET_KEY_PRESS, OP_SYS_GET_CPU_VER, OP_SYS_WAIT, OP_SYS_TIME_REG,

    // Disk Standard Library
    OP_DISK_GET_SIZE_REG,
    OP_DISK_READ_SECTOR_MEM_REG_REG,
    OP_DISK_WRITE_SECTOR_MEM_REG_REG,
    OP_DISK_CREATE_IMAGE,
    OP_DISK_FORMAT_DISK,
    OP_DISK_GET_VOLUME_LABEL_MEM,
    OP_DISK_SET_VOLUME_LABEL_MEM,

    OP_INVALID
} Opcode;

typedef enum {
    REG_R0, REG_R1, REG_R2, REG_R3, REG_R4, REG_R5, REG_R6, REG_R7,
    REG_R8, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15,
    REG_R16, REG_R17, REG_R18, REG_R19, REG_R20, REG_R21, REG_R22, REG_R23,
    REG_R24, REG_R25, REG_R26, REG_R27, REG_R28, REG_R29, REG_R30, REG_R31,
    REG_SP,
    REG_ZF, REG_SF, REG_CF, REG_OF,
    REG_FLAG_COUNT,
    REG_INVALID
} RegisterIndex;

#define NUM_FLAG_REGISTERS 4
#define NUM_TOTAL_REGISTERS (NUM_GENERAL_REGISTERS + NUM_FLAG_REGISTERS)

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

typedef struct {
    char name[32];
    uint32_t address;
    uint32_t size;
} BufferDefinition;

uint8_t memory[MEMORY_SIZE];
double registers[NUM_TOTAL_REGISTERS];
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
BufferDefinition buffers[MAX_BUFFERS];
int buffer_count = 0;
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

typedef enum {
    DISK_OK = 0,
    DISK_ERROR = 1,
    DISK_NOT_FOUND = 2,
    DISK_INVALID_SECTOR = 3,
    DISK_INVALID_OFFSET = 4,
    DISK_FULL = 5,
    DISK_EMPTY = 6,
    DISK_READ_ERROR = 7,
    DISK_WRITE_ERROR = 8,
    DISK_SEEK_ERROR = 9,
    DISK_HEADER_ERROR = 10,
    DISK_CREATE_ERROR = 11,
    DISK_OPEN_ERROR = 12,
    DISK_CLOSE_ERROR = 13,
    DISK_PARAM_ERROR = 14
} DiskResultCode;

#define DISK_IMAGE_FILENAME "img/drive.img"
#define DISK_SECTOR_SIZE 512
#define DISK_SIZE_MB 32
#define DISK_SIZE_BYTES (DISK_SIZE_MB * 1024 * 1024)
#define DISK_NUM_SECTORS (DISK_SIZE_BYTES / DISK_SECTOR_SIZE)
#define DISK_HEADER_SECTOR 0

typedef struct {
    uint32_t magic_number;
    uint32_t version;
    uint32_t sector_size;
    uint32_t num_sectors;
    char     volume_label[32];
    uint8_t  reserved[DISK_SECTOR_SIZE - 32 - 4 * 3];
} DiskHeader;

#define DISK_MAGIC_NUMBER 0x12345678
#define DISK_VERSION 1

DiskResultCode disk_get_size(uint32_t* size_bytes);
DiskResultCode disk_read_sector(uint32_t sector_number, uint32_t address_mem, uint32_t count);
DiskResultCode disk_write_sector(uint32_t sector_number, uint32_t address_mem, uint32_t count);
DiskResultCode disk_get_volume_label(uint32_t address_mem);
DiskResultCode disk_set_volume_label(uint32_t address_mem);
DiskResultCode create_disk_image();

// System Library Functions

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
    tv.tv_usec = 0;

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

void sys_print_char(char character) {
    printf("%c", character);
}

void sys_print_newline() {
    sys_print_char('\n');
    cursor_y++;
    cursor_x = 0;
}

void sys_read_string(uint32_t address, uint32_t max_len) {
    if (address >= MEMORY_SIZE) {
        printf("Error: READ_STRING address out of bounds.\n");
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
        else if (c == 127) {
            if (i > 0) {
                i--;
                str_ptr[i] = '\0';
                printf("\b \b");
            }
        }
        else if (c >= 32 && c <= 126) {
            str_ptr[i++] = c;
            sys_print_char(c);
        }
        else if (c == 27) {
            continue;
        }
    }
    if (i == max_len - 1) {
        str_ptr[i] = '\0';
    }
}

void sys_print_number_dec(double number) {
    printf("%f", number);
}

void sys_print_number_hex(uint32_t number) {
    printf("0x%X", number);
}

void sys_number_to_string(uint32_t number, uint32_t address, uint32_t buffer_size) {
    if (address >= MEMORY_SIZE || address + buffer_size > MEMORY_SIZE) {
        printf("Error: NUMBER_TO_STRING buffer out of bounds.\n");
        return;
    }
    char* str_ptr = (char*)&memory[address];
    snprintf(str_ptr, buffer_size, "%u", number);
}

void sys_set_cursor_pos(uint32_t x, uint32_t y) {
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

void sys_get_cursor_pos(uint32_t* x, uint32_t* y) {
    *x = cursor_x;
    *y = cursor_y;
}

void sys_set_text_color(uint32_t color_code) {
    text_color = color_code;
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)(color_code % 16));
#else
    if (color_code >= 0 && color_code <= 255) {
        printf("\033[38;5;%dm", color_code);
    }
    else {
        printf("\033[37m");
    }
#endif
}

void sys_reset_text_color() {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#else
    printf("\033[0m");
#endif
    text_color = 7;
}

void sys_print_string(uint32_t address) {
    if (address >= MEMORY_SIZE) {
        printf("Error: PRINT_STRING address out of bounds.\n");
        return;
    }
    char* str = (char*)&memory[address];
    while (*str != '\0') {
        sys_print_char(*str);
        str++;
        if ((uint32_t)(str - (char*)memory) >= MEMORY_SIZE) {
            printf("Error: PRINT_STRING string exceeds memory bounds.\n");
            return;
        }
    }
}

void sys_clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
    printf("\033[H");
#endif
    cursor_x = 0;
    cursor_y = 0;
}

void sys_wait(uint32_t milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

uint32_t sys_get_cpu_ver() {
    return CPU_VER;
}

double sys_time() {
    return (double)time(NULL);
}

DiskResultCode disk_get_size(uint32_t* size_bytes) {
    FILE* disk_image_file = fopen(DISK_IMAGE_FILENAME, "rb");
    if (!disk_image_file) {
        return DISK_NOT_FOUND;
    }
    DiskHeader header;
    if (fread(&header, sizeof(DiskHeader), 1, disk_image_file) != 1) {
        fclose(disk_image_file);
        return DISK_READ_ERROR;
    }
    fclose(disk_image_file);

    if (header.magic_number != DISK_MAGIC_NUMBER || header.version != DISK_VERSION) {
        return DISK_HEADER_ERROR;
    }

    *size_bytes = DISK_SIZE_BYTES;
    return DISK_OK;
}

DiskResultCode disk_read_sector(uint32_t sector_number, uint32_t address_mem, uint32_t count) {
    if (sector_number >= DISK_NUM_SECTORS) {
        return DISK_INVALID_SECTOR;
    }
    if (address_mem >= MEMORY_SIZE || (address_mem + (count * DISK_SECTOR_SIZE)) > MEMORY_SIZE) {
        return DISK_INVALID_OFFSET;
    }
    if (count == 0) return DISK_PARAM_ERROR;
    if (count > (DISK_NUM_SECTORS - sector_number)) count = (DISK_NUM_SECTORS - sector_number);

    FILE* disk_image_file = fopen(DISK_IMAGE_FILENAME, "rb");
    if (!disk_image_file) {
        return DISK_NOT_FOUND;
    }

    if (fseek(disk_image_file, (sector_number * DISK_SECTOR_SIZE), SEEK_SET) != 0) {
        fclose(disk_image_file);
        return DISK_SEEK_ERROR;
    }

    size_t bytes_to_read = count * DISK_SECTOR_SIZE;
    size_t bytes_read = fread(&memory[address_mem], 1, bytes_to_read, disk_image_file);
    fclose(disk_image_file);

    if (bytes_read != bytes_to_read) {
        return DISK_READ_ERROR;
    }

    return DISK_OK;
}

DiskResultCode disk_write_sector(uint32_t sector_number, uint32_t address_mem, uint32_t count) {
    if (sector_number >= DISK_NUM_SECTORS) {
        return DISK_INVALID_SECTOR;
    }
    if (address_mem >= MEMORY_SIZE || (address_mem + (count * DISK_SECTOR_SIZE)) > MEMORY_SIZE) {
        return DISK_INVALID_OFFSET;
    }
    if (count == 0) return DISK_PARAM_ERROR;
    if (count > (DISK_NUM_SECTORS - sector_number)) count = (DISK_NUM_SECTORS - sector_number);

    FILE* disk_image_file = fopen(DISK_IMAGE_FILENAME, "rb+");
    if (!disk_image_file) {
        return DISK_NOT_FOUND;
    }

    if (fseek(disk_image_file, (sector_number * DISK_SECTOR_SIZE), SEEK_SET) != 0) {
        fclose(disk_image_file);
        return DISK_SEEK_ERROR;
    }

    size_t bytes_to_write = count * DISK_SECTOR_SIZE;
    size_t bytes_written = fwrite(&memory[address_mem], 1, bytes_to_write, disk_image_file);
    fclose(disk_image_file);

    if (bytes_written != bytes_to_write) {
        return DISK_WRITE_ERROR;
    }

    return DISK_OK;
}

DiskResultCode disk_get_volume_label(uint32_t address_mem) {
    if (address_mem >= MEMORY_SIZE || (address_mem + 32) > MEMORY_SIZE) {
        return DISK_INVALID_OFFSET;
    }

    FILE* disk_image_file = fopen(DISK_IMAGE_FILENAME, "rb");
    if (!disk_image_file) {
        return DISK_NOT_FOUND;
    }
    DiskHeader header;
    if (fread(&header, sizeof(DiskHeader), 1, disk_image_file) != 1) {
        fclose(disk_image_file);
        return DISK_READ_ERROR;
    }
    fclose(disk_image_file);
    if (header.magic_number != DISK_MAGIC_NUMBER || header.version != DISK_VERSION) {
        return DISK_HEADER_ERROR;
    }

    strncpy((char*)&memory[address_mem], header.volume_label, 32);
    ((char*)&memory[address_mem])[31] = '\0';

    return DISK_OK;
}

DiskResultCode disk_set_volume_label(uint32_t address_mem) {
    if (address_mem >= MEMORY_SIZE || (address_mem + 32) > MEMORY_SIZE) {
        return DISK_INVALID_OFFSET;
    }

    FILE* disk_image_file = fopen(DISK_IMAGE_FILENAME, "rb+");
    if (!disk_image_file) {
        return DISK_NOT_FOUND;
    }
    DiskHeader header;
    if (fread(&header, sizeof(DiskHeader), 1, disk_image_file) != 1) {
        fclose(disk_image_file);
        return DISK_READ_ERROR;
    }
    if (header.magic_number != DISK_MAGIC_NUMBER || header.version != DISK_VERSION) {
        fclose(disk_image_file);
        return DISK_HEADER_ERROR;
    }

    strncpy(header.volume_label, (char*)&memory[address_mem], 32);
    header.volume_label[31] = '\0';

    if (fseek(disk_image_file, 0, SEEK_SET) != 0) {
        fclose(disk_image_file);
        return DISK_SEEK_ERROR;
    }
    if (fwrite(&header, sizeof(DiskHeader), 1, disk_image_file) != 1) {
        fclose(disk_image_file);
        return DISK_WRITE_ERROR;
    }

    fclose(disk_image_file);
    return DISK_OK;
}

DiskResultCode create_disk_image() {
    FILE* disk_image_file = fopen(DISK_IMAGE_FILENAME, "wb+");
    if (!disk_image_file) {
        perror("Error creating disk image file");
        return DISK_CREATE_ERROR;
    }

    DiskHeader header;
    header.magic_number = DISK_MAGIC_NUMBER;
    header.version = DISK_VERSION;
    header.sector_size = DISK_SECTOR_SIZE;
    header.num_sectors = DISK_NUM_SECTORS;
    strncpy(header.volume_label, "VIRTUAL_DRIVE", sizeof(header.volume_label) - 1);
    header.volume_label[sizeof(header.volume_label) - 1] = '\0';
    memset(header.reserved, 0, sizeof(header.reserved));


    if (fwrite(&header, sizeof(DiskHeader), 1, disk_image_file) != 1) {
        fclose(disk_image_file);
        remove(DISK_IMAGE_FILENAME);
        perror("Error writing disk header");
        return DISK_WRITE_ERROR;
    }

    size_t data_size = DISK_SIZE_BYTES - DISK_SECTOR_SIZE;
    char zero_buffer[4096] = { 0 };
    size_t sectors_to_write = data_size / sizeof(zero_buffer);
    for (size_t i = 0; i < sectors_to_write; ++i) {
        if (fwrite(zero_buffer, sizeof(zero_buffer), 1, disk_image_file) != 1) {
            fclose(disk_image_file);
            remove(DISK_IMAGE_FILENAME);
            perror("Error writing disk data");
            return DISK_WRITE_ERROR;
        }
    }
    size_t remaining_bytes = data_size % sizeof(zero_buffer);
    if (remaining_bytes > 0) {
        if (fwrite(zero_buffer, remaining_bytes, 1, disk_image_file) != 1) {
            fclose(disk_image_file);
            remove(DISK_IMAGE_FILENAME);
            perror("Error writing remaining disk data");
            return DISK_WRITE_ERROR;
        }
    }


    fclose(disk_image_file);
    printf("Disk image '%s' created successfully.\n", DISK_IMAGE_FILENAME);
    return DISK_OK;
}

// Instruction Decoding

Opcode decode_opcode() {
    if (program_counter >= MEMORY_SIZE) return OP_INVALID;
    return (Opcode)memory[program_counter++];
}

RegisterIndex decode_register() {
    if (program_counter >= MEMORY_SIZE) return REG_INVALID;
    uint8_t reg_index = memory[program_counter++];
    if (reg_index >= NUM_TOTAL_REGISTERS) return REG_INVALID;
    return (RegisterIndex)reg_index;
}

double decode_value_double() {
    if (program_counter + 8 > MEMORY_SIZE) return 0.0;
    double value = *(double*)&memory[program_counter];
    program_counter += 8;
    return value;
}

uint32_t decode_value_uint32() {
    if (program_counter + 4 > MEMORY_SIZE) return 0;
    uint32_t value = *(uint32_t*)&memory[program_counter];
    program_counter += 4;
    return value;
}

uint32_t decode_address() {
    return decode_value_uint32();
}

// Flag Setting

void set_zero_flag_float(double result) {
    registers[REG_ZF] = (fabs(result) < 1e-9);
}

void set_sign_flag_float(double result) {
    registers[REG_SF] = (result < 0.0);
}

void set_carry_flag_float(double result, double operand1, double operand2, Opcode opcode) {
    if (opcode == OP_ADD_REG_REG || opcode == OP_ADD_REG_VAL || opcode == OP_MATH_ADD) {
        registers[REG_CF] = (result > HUGE_VAL || result < -HUGE_VAL);
    }
    else if (opcode == OP_SUB_REG_REG || opcode == OP_SUB_REG_VAL || opcode == OP_MATH_SUB) {
        registers[REG_CF] = (result > HUGE_VAL || result < -HUGE_VAL);
    }
    else {
        registers[REG_CF] = 0;
    }
}

void set_overflow_flag_float(double result, double operand1, double operand2, Opcode opcode) {
    registers[REG_OF] = 0;
}

void set_zero_flag_int(uint32_t result) {
    registers[REG_ZF] = (result == 0);
}

void set_sign_flag_int(uint32_t result) {
    registers[REG_SF] = ((int32_t)result < 0);
}

void set_carry_flag_int(uint32_t result, uint32_t operand1, uint32_t operand2, Opcode opcode) {
    if (opcode == OP_ADD_REG_REG || opcode == OP_ADD_REG_VAL) {
        registers[REG_CF] = (result < operand1);
    }
    else if (opcode == OP_SUB_REG_REG || opcode == OP_SUB_REG_VAL) {
        registers[REG_CF] = (result > operand1);
    }
    else if (opcode >= OP_SHL_REG_REG && opcode <= OP_ROR_REG_VAL) {
        registers[REG_CF] = 0;
    }
    else {
        registers[REG_CF] = 0;
    }
}

void set_overflow_flag_int(uint32_t result, uint32_t operand1, uint32_t operand2, Opcode opcode) {
    if (opcode == OP_ADD_REG_REG || opcode == OP_ADD_REG_VAL) {
        registers[REG_OF] = (((operand1 ^ operand2) & 0x80000000) == 0 && ((operand1 ^ result) & 0x80000000) != 0);
    }
    else if (opcode == OP_SUB_REG_REG || opcode == OP_SUB_REG_VAL) {
        registers[REG_OF] = (((operand1 ^ operand2) & 0x80000000) != 0 && ((operand1 ^ result) & 0x80000000) != 0);
    }
    else {
        registers[REG_OF] = 0;
    }
}

// CPU Instruction Execution
void execute_instruction(Opcode opcode) {
    RegisterIndex reg1, reg2, reg3, reg_dest, reg_src;
    double value_double;
    uint32_t value_uint32, address, count;

    switch (opcode) {
    case OP_NOP: break;
    case OP_MOV_REG_REG: {
        reg_dest = decode_register();
        reg_src = decode_register();
        if (reg_dest != REG_INVALID && reg_src != REG_INVALID) {
            registers[reg_dest] = registers[reg_src];
        }
        break;
    }
    case OP_MOV_REG_VAL: {
        reg_dest = decode_register();
        value_double = decode_value_double();
        if (reg_dest != REG_INVALID) {
            registers[reg_dest] = value_double;
        }
        break;
    }
    case OP_MOV_REG_MEM: {
        reg1 = decode_register();
        address = decode_address();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE - 8) registers[reg1] = *(double*)&memory[address];
        break;
    }
    case OP_MOV_MEM_REG: {
        address = decode_address();
        reg1 = decode_register();
        if (reg1 != REG_INVALID && address < MEMORY_SIZE - 8) *(double*)&memory[address] = registers[reg1];
        break;
    }
    case OP_ADD_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] + registers[reg2];
            set_carry_flag_float(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag_float(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_ADD_REG_VAL: {
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double result = registers[reg1] + value_double;
            set_carry_flag_float(result, registers[reg1], value_double, opcode);
            set_overflow_flag_float(result, registers[reg1], value_double, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_SUB_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] - registers[reg2];
            set_carry_flag_float(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag_float(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_SUB_REG_VAL: {
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double result = registers[reg1] - value_double;
            set_carry_flag_float(result, registers[reg1], value_double, opcode);
            set_overflow_flag_float(result, registers[reg1], value_double, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_MUL_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = registers[reg1] * registers[reg2];
            set_carry_flag_float(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag_float(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_MUL_REG_VAL: {
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            double result = registers[reg1] * value_double;
            set_carry_flag_float(result, registers[reg1], value_double, opcode);
            set_overflow_flag_float(result, registers[reg1], value_double, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_DIV_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = registers[reg1] / registers[reg2];
                set_carry_flag_float(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag_float(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Error: Division by zero!\n");
                running = false;
            }
        }
        break;
    }
    case OP_DIV_REG_VAL: {
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            if (fabs(value_double) > 1e-9) {
                double result = registers[reg1] / value_double;
                set_carry_flag_float(result, registers[reg1], value_double, opcode);
                set_overflow_flag_float(result, registers[reg1], value_double, opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Error: Division by zero!\n");
                running = false;
            }
        }
        break;
    }
    case OP_MOD_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (fabs(registers[reg2]) > 1e-9) {
                double result = fmod(registers[reg1], registers[reg2]);
                set_carry_flag_float(result, registers[reg1], registers[reg2], opcode);
                set_overflow_flag_float(result, registers[reg1], registers[reg2], opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Error: Modulo by zero!\n");
                running = false;
            }
        }
        break;
    }
    case OP_MOD_REG_VAL: {
        reg1 = decode_register();
        value_double = decode_value_double();
        if (reg1 != REG_INVALID) {
            if (fabs(value_double) > 1e-9) {
                double result = fmod(registers[reg1], value_double);
                set_carry_flag_float(result, registers[reg1], value_double, opcode);
                set_overflow_flag_float(result, registers[reg1], value_double, opcode);
                registers[reg1] = result;
                set_zero_flag_float(registers[reg1]);
                set_sign_flag_float(registers[reg1]);
            }
            else {
                printf("Error: Modulo by zero!\n");
                running = false;
            }
        }
        break;
    }
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
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = value_uint32;
            uint32_t result = val1 & val2;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
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
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = value_uint32;
            uint32_t result = val1 | val2;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
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
        value_uint32 = decode_value_uint32();
        if (reg1 != REG_INVALID) {
            uint32_t val1 = (uint32_t)registers[reg1];
            uint32_t val2 = value_uint32;
            uint32_t result = val1 ^ val2;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
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
            set_carry_flag_int(result, val1, 0, opcode);
            set_overflow_flag_int(result, val1, 0, opcode);
        }
        break;
    }
    case OP_NEG_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            int32_t val1 = (int32_t)registers[reg1];
            int32_t result = -val1;
            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, 0, opcode);
            set_overflow_flag_int(result, val1, 0, opcode);
        }
        break;
    }
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
            uint32_t val2 = value_uint32;
            uint32_t result = val1 & val2;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_SHL_REG_REG:
    case OP_SHL_REG_VAL: {
        uint32_t val1, val2, result = 0;
        reg1 = decode_register();
        if (opcode == OP_SHL_REG_VAL) value_uint32 = decode_value_uint32();
        else reg2 = decode_register();

        if (reg1 != REG_INVALID) {
            val1 = (uint32_t)registers[reg1];
            val2 = (opcode == OP_SHL_REG_VAL) ? value_uint32 : (uint32_t)registers[reg2];
            result = val1 << (val2 & 0x1F);

            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_SHR_REG_REG:
    case OP_SHR_REG_VAL: {
        uint32_t val1, val2, result = 0;
        reg1 = decode_register();
        if (opcode == OP_SHR_REG_VAL) value_uint32 = decode_value_uint32();
        else reg2 = decode_register();

        if (reg1 != REG_INVALID) {
            val1 = (uint32_t)registers[reg1];
            val2 = (opcode == OP_SHR_REG_VAL) ? value_uint32 : (uint32_t)registers[reg2];
            result = val1 >> (val2 & 0x1F);

            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_SAR_REG_REG:
    case OP_SAR_REG_VAL: {
        uint32_t val1, val2, result = 0;
        reg1 = decode_register();
        if (opcode == OP_SAR_REG_VAL) value_uint32 = decode_value_uint32();
        else reg2 = decode_register();

        if (reg1 != REG_INVALID) {
            val1 = (uint32_t)registers[reg1];
            val2 = (opcode == OP_SAR_REG_VAL) ? value_uint32 : (uint32_t)registers[reg2];
            result = (int32_t)val1 >> (val2 & 0x1F);

            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_ROL_REG_REG:
    case OP_ROL_REG_VAL: {
        uint32_t val1, val2, result = 0;
        reg1 = decode_register();
        if (opcode == OP_ROL_REG_VAL) value_uint32 = decode_value_uint32();
        else reg2 = decode_register();

        if (reg1 != REG_INVALID) {
            val1 = (uint32_t)registers[reg1];
            val2 = (opcode == OP_ROL_REG_VAL) ? value_uint32 : (uint32_t)registers[reg2];
            uint32_t bits = val2 & 0x1F;
            result = (val1 << bits) | (val1 >> (32 - bits));

            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_ROR_REG_REG:
    case OP_ROR_REG_VAL: {
        uint32_t val1, val2, result = 0;
        reg1 = decode_register();
        if (opcode == OP_ROR_REG_VAL) value_uint32 = decode_value_uint32();
        else reg2 = decode_register();

        if (reg1 != REG_INVALID) {
            val1 = (uint32_t)registers[reg1];
            val2 = (opcode == OP_ROR_REG_VAL) ? value_uint32 : (uint32_t)registers[reg2];
            uint32_t bits = val2 & 0x1F;
            result = (val1 >> bits) | (val1 << (32 - bits));

            registers[reg1] = (double)result;
            set_zero_flag_int(result);
            set_sign_flag_int(result);
            set_carry_flag_int(result, val1, val2, opcode);
            set_overflow_flag_int(result, val1, val2, opcode);
        }
        break;
    }
    case OP_CMP_REG_REG:
    case OP_CMP_REG_VAL: {
        reg1 = decode_register();
        if (opcode == OP_CMP_REG_VAL) value_double = decode_value_double();
        else reg2 = decode_register();

        if (reg1 != REG_INVALID) {
            double val1 = registers[reg1];
            double val2 = (opcode == OP_CMP_REG_VAL) ? value_double : registers[reg2];
            set_zero_flag_float(val1 - val2);
            set_sign_flag_float(val1 - val2);
            set_carry_flag_float(val1 - val2, val1, val2, opcode);
            set_overflow_flag_float(val1 - val2, val1, val2, opcode);
        }
        break;
    }
    case OP_IMUL_REG_REG:
    case OP_IDIV_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            if (opcode == OP_IMUL_REG_REG) registers[reg1] = (double)((int32_t)registers[reg1] * (int32_t)registers[reg2]);
            else if (opcode == OP_IDIV_REG_REG) {
                if ((int32_t)registers[reg2] != 0) registers[reg1] = (double)((int32_t)registers[reg1] / (int32_t)registers[reg2]);
                else { printf("Error: Signed division by zero!\n"); running = false; }
            }
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_carry_flag_float(registers[reg1], registers[reg2], 0, opcode);
            set_overflow_flag_float(registers[reg1], registers[reg2], 0, opcode);
        }
        break;
    }
    case OP_MOVZX_REG_REG:
    case OP_MOVZX_REG_MEM:
    case OP_MOVSX_REG_REG:
    case OP_MOVSX_REG_MEM:
    case OP_LEA_REG_MEM: {
        reg_dest = decode_register();
        if (opcode == OP_MOVZX_REG_REG || opcode == OP_MOVSX_REG_REG) reg_src = decode_register();
        else address = decode_address();

        if (reg_dest != REG_INVALID) {
            if (opcode == OP_MOVZX_REG_REG) registers[reg_dest] = (double)(uint32_t)registers[reg_src];
            else if (opcode == OP_MOVZX_REG_MEM && address < MEMORY_SIZE - 4) registers[reg_dest] = (double)*(uint32_t*)&memory[address];
            else if (opcode == OP_MOVSX_REG_REG) registers[reg_dest] = (double)(int32_t)registers[reg_src];
            else if (opcode == OP_MOVSX_REG_MEM && address < MEMORY_SIZE - 4) registers[reg_dest] = (double)*(int32_t*)&memory[address];
            else if (opcode == OP_LEA_REG_MEM) registers[reg_dest] = (double)address;
        }
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
        address = decode_address();
        bool jump = false;
        if (opcode == OP_JMP) jump = true;
        else if (opcode == OP_JMP_NZ && !registers[REG_ZF]) jump = true;
        else if (opcode == OP_JMP_Z && registers[REG_ZF]) jump = true;
        else if (opcode == OP_JMP_S && registers[REG_SF]) jump = true;
        else if (opcode == OP_JMP_NS && !registers[REG_SF]) jump = true;
        else if (opcode == OP_JMP_C && registers[REG_CF]) jump = true;
        else if (opcode == OP_JMP_NC && !registers[REG_CF]) jump = true;
        else if (opcode == OP_JMP_O && registers[REG_OF]) jump = true;
        else if (opcode == OP_JMP_NO && !registers[REG_OF]) jump = true;
        else if (opcode == OP_JMP_GE && !(registers[REG_SF])) jump = true;
        else if (opcode == OP_JMP_LE && (registers[REG_ZF] || registers[REG_SF])) jump = true;
        else if (opcode == OP_JMP_G && (!(registers[REG_ZF]) && !(registers[REG_SF]))) jump = true;
        else if (opcode == OP_JMP_L && (!(registers[REG_ZF]) && registers[REG_SF])) jump = true;

        if (jump) program_counter = address;
        if (opcode == OP_CALL_ADDR) {
            registers[REG_SP] -= 8;
            if ((int32_t)registers[REG_SP] < 0) { printf("Stack Overflow during CALL!\n"); running = false; break; }
            *(double*)&memory[(uint32_t)registers[REG_SP]] = (double)program_counter;
            program_counter = address;
        }
        break;
    }
    case OP_HLT: running = false; break;
    case OP_INC_REG:
    case OP_DEC_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            if (opcode == OP_INC_REG) registers[reg1]++;
            else registers[reg1]--;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
            set_overflow_flag_float(registers[reg1], 0, 0, opcode);
            set_carry_flag_float(registers[reg1], 0, 0, opcode);
        }
        break;
    }
    case OP_INC_MEM:
    case OP_DEC_MEM: {
        address = decode_address();
        if (address < MEMORY_SIZE - 8) {
            double val = *(double*)&memory[address];
            if (opcode == OP_INC_MEM) val++;
            else val--;
            *(double*)&memory[address] = val;
        }
        break;
    }
    case OP_RND_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) registers[reg1] = (double)rand();
        break;
    }
    case OP_PUSH_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[REG_SP] -= 8;
            if ((int32_t)registers[REG_SP] < 0) { printf("Stack Overflow!\n"); running = false; break; }
            *(double*)&memory[(uint32_t)registers[REG_SP]] = registers[reg1];
        }
        break;
    }
    case OP_POP_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            if ((uint32_t)registers[REG_SP] >= MEMORY_SIZE) { printf("Stack Underflow!\n"); running = false; break; }
            registers[reg1] = *(double*)&memory[(uint32_t)registers[REG_SP]];
            registers[REG_SP] += 8;
        }
        break;
    }
    case OP_RET: {
        if ((uint32_t)registers[REG_SP] >= MEMORY_SIZE) { printf("Stack Underflow during RET!\n"); running = false; break; }
        program_counter = (uint32_t) * (double*)&memory[(uint32_t)registers[REG_SP]];
        registers[REG_SP] += 8;
        break;
    }
    case OP_XCHG_REG_REG: {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double temp = registers[reg1];
            registers[reg1] = registers[reg2];
            registers[reg2] = temp;
        }
        break;
    }
    case OP_BSWAP_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            uint32_t val = (uint32_t)registers[reg1];
            uint32_t bswap_val = ((val >> 24) & 0x000000FF) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | ((val << 24) & 0xFF000000);
            registers[reg1] = (double)bswap_val;
        }
        break;
    }
    case OP_SETZ_REG:
    case OP_SETNZ_REG: {
        reg1 = decode_register();
        if (reg1 != REG_INVALID) {
            registers[reg1] = (opcode == OP_SETZ_REG) ? registers[REG_ZF] : !registers[REG_ZF];
        }
        break;
    }

                     // Math Standard Library Implementation
    case OP_MATH_ADD: case OP_MATH_SUB: case OP_MATH_MUL: case OP_MATH_DIV: case OP_MATH_MOD:
    case OP_MATH_POW: case OP_MATH_MIN: case OP_MATH_MAX: case OP_MATH_ATAN2:
    {
        reg1 = decode_register();
        reg2 = decode_register();
        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            double result = 0;
            if (opcode == OP_MATH_ADD) result = registers[reg1] + registers[reg2];
            else if (opcode == OP_MATH_SUB) result = registers[reg1] - registers[reg2];
            else if (opcode == OP_MATH_MUL) result = registers[reg1] * registers[reg2];
            else if (opcode == OP_MATH_DIV) { if (fabs(registers[reg2]) > 1e-9) result = registers[reg1] / registers[reg2]; else { printf("Math Error: Division by zero!\n"); running = false; break; } }
            else if (opcode == OP_MATH_MOD) { if (fabs(registers[reg2]) > 1e-9) result = fmod(registers[reg1], registers[reg2]); else { printf("Math Error: Modulo by zero!\n"); running = false; break; } }
            else if (opcode == OP_MATH_POW) result = pow(registers[reg1], registers[reg2]);
            else if (opcode == OP_MATH_MIN) result = (registers[reg1] < registers[reg2]) ? registers[reg1] : registers[reg2];
            else if (opcode == OP_MATH_MAX) result = (registers[reg1] > registers[reg2]) ? registers[reg1] : registers[reg2];
            else if (opcode == OP_MATH_ATAN2) result = atan2(registers[reg1], registers[reg2]);

            set_carry_flag_float(result, registers[reg1], registers[reg2], opcode);
            set_overflow_flag_float(result, registers[reg1], registers[reg2], opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }
    case OP_MATH_ABS: case OP_MATH_SIN: case OP_MATH_COS: case OP_MATH_TAN: case OP_MATH_ASIN:
    case OP_MATH_ACOS: case OP_MATH_ATAN: case OP_MATH_SQRT: case OP_MATH_LOG: case OP_MATH_EXP:
    case OP_MATH_FLOOR: case OP_MATH_CEIL: case OP_MATH_ROUND: case OP_MATH_NEG: case OP_MATH_LOG10:
    case OP_MATH_CLAMP: case OP_MATH_LERP:
    {
        reg1 = decode_register();
        if (opcode == OP_MATH_CLAMP) {
            reg2 = decode_register();
            reg3 = decode_register();
            if (reg1 != REG_INVALID && reg2 != REG_INVALID && reg3 != REG_INVALID) {
                registers[reg1] = fmax(registers[reg2], fmin(registers[reg1], registers[reg3]));
            }
            break;
        }
        else if (opcode == OP_MATH_LERP) {
            reg2 = decode_register();
            reg3 = decode_register();
            RegisterIndex reg4 = decode_register();
            if (reg1 != REG_INVALID && reg2 != REG_INVALID && reg3 != REG_INVALID && reg4 != REG_INVALID) {
                registers[reg1] = registers[reg2] + (registers[reg3] - registers[reg2]) * registers[reg4];
            }
            break;
        }

        if (reg1 != REG_INVALID) {
            double result = 0;
            if (opcode == OP_MATH_ABS) result = fabs(registers[reg1]);
            else if (opcode == OP_MATH_SIN) result = sin(registers[reg1]);
            else if (opcode == OP_MATH_COS) result = cos(registers[reg1]);
            else if (opcode == OP_MATH_TAN) result = tan(registers[reg1]);
            else if (opcode == OP_MATH_ASIN) result = asin(registers[reg1]);
            else if (opcode == OP_MATH_ACOS) result = acos(registers[reg1]);
            else if (opcode == OP_MATH_ATAN) result = atan(registers[reg1]);
            else if (opcode == OP_MATH_SQRT) { if (registers[reg1] >= 0) result = sqrt(registers[reg1]); else { printf("Math Error: Sqrt of negative!\n"); running = false; break; } }
            else if (opcode == OP_MATH_LOG) { if (registers[reg1] > 0) result = log(registers[reg1]); else { printf("Math Error: Log of non-positive!\n"); running = false; break; } }
            else if (opcode == OP_MATH_EXP) result = exp(registers[reg1]);
            else if (opcode == OP_MATH_FLOOR) result = floor(registers[reg1]);
            else if (opcode == OP_MATH_CEIL) result = ceil(registers[reg1]);
            else if (opcode == OP_MATH_ROUND) result = round(registers[reg1]);
            else if (opcode == OP_MATH_NEG) result = -registers[reg1];
            else if (opcode == OP_MATH_LOG10) { if (registers[reg1] > 0) result = log10(registers[reg1]); else { printf("Math Error: Log10 of non-positive!\n"); running = false; break; } }

            set_carry_flag_float(result, registers[reg1], 0, opcode);
            set_overflow_flag_float(result, registers[reg1], 0, opcode);
            registers[reg1] = result;
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        break;
    }

    // String Standard Library Implementation
    case OP_STR_LEN_REG_MEM: case OP_STR_CPY_MEM_MEM: case OP_STR_CAT_MEM_MEM: case OP_STR_CMP_REG_MEM_MEM:
    case OP_STR_NCPY_MEM_MEM_REG: case OP_STR_NCAT_MEM_MEM_REG: case OP_STR_TOUPPER_MEM: case OP_STR_TOLOWER_MEM:
    case OP_STR_CHR_REG_MEM_VAL: case OP_STR_STR_REG_MEM_MEM: case OP_STR_ATOI_REG_MEM: case OP_STR_ITOA_MEM_REG_REG:
    case OP_STR_SUBSTR_MEM_MEM_REG_REG: case OP_STR_FMT_MEM_MEM_REG_REG:
    {
        if (opcode == OP_STR_LEN_REG_MEM) {
            reg1 = decode_register(); address = decode_address();
            if (reg1 != REG_INVALID && address < MEMORY_SIZE) registers[reg1] = (double)strlen((char*)&memory[address]);
        }
        else if (opcode == OP_STR_CPY_MEM_MEM) {
            uint32_t dest_addr = decode_address(); uint32_t src_addr = decode_address();
            if (dest_addr < MEMORY_SIZE && src_addr < MEMORY_SIZE) strcpy((char*)&memory[dest_addr], (char*)&memory[src_addr]);
        }
        else if (opcode == OP_STR_CAT_MEM_MEM) {
            uint32_t dest_addr = decode_address(); uint32_t src_addr = decode_address();
            if (dest_addr < MEMORY_SIZE && src_addr < MEMORY_SIZE) strcat((char*)&memory[dest_addr], (char*)&memory[src_addr]);
        }
        else if (opcode == OP_STR_CMP_REG_MEM_MEM) {
            reg1 = decode_register(); uint32_t addr1 = decode_address(); uint32_t addr2 = decode_address();
            if (reg1 != REG_INVALID && addr1 < MEMORY_SIZE && addr2 < MEMORY_SIZE) registers[reg1] = (double)strcmp((char*)&memory[addr1], (char*)&memory[addr2]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        else if (opcode == OP_STR_NCPY_MEM_MEM_REG) {
            uint32_t dest_addr = decode_address(); uint32_t src_addr = decode_address(); reg1 = decode_register();
            if (dest_addr < MEMORY_SIZE && src_addr < MEMORY_SIZE && reg1 != REG_INVALID) strncpy((char*)&memory[dest_addr], (char*)&memory[src_addr], (uint32_t)registers[reg1]);
        }
        else if (opcode == OP_STR_NCAT_MEM_MEM_REG) {
            uint32_t dest_addr = decode_address(); uint32_t src_addr = decode_address(); reg1 = decode_register();
            if (dest_addr < MEMORY_SIZE && src_addr < MEMORY_SIZE && reg1 != REG_INVALID) strncat((char*)&memory[dest_addr], (char*)&memory[src_addr], (uint32_t)registers[reg1]);
        }
        else if (opcode == OP_STR_TOUPPER_MEM) {
            address = decode_address(); if (address < MEMORY_SIZE) { char* str = (char*)&memory[address]; while (*str) { *str = toupper((unsigned char)*str); str++; } }
        }
        else if (opcode == OP_STR_TOLOWER_MEM) {
            address = decode_address(); if (address < MEMORY_SIZE) { char* str = (char*)&memory[address]; while (*str) { *str = tolower((unsigned char)*str); str++; } }
        }
        else if (opcode == OP_STR_CHR_REG_MEM_VAL) {
            reg1 = decode_register(); address = decode_address(); value_uint32 = decode_value_uint32();
            if (reg1 != REG_INVALID && address < MEMORY_SIZE) { char* res = strchr((char*)&memory[address], (char)value_uint32); registers[reg1] = (double)(res ? res - (char*)&memory[address] : -1); }
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        else if (opcode == OP_STR_STR_REG_MEM_MEM) {
            reg1 = decode_register(); uint32_t addr1 = decode_address(); uint32_t addr2 = decode_address();
            if (reg1 != REG_INVALID && addr1 < MEMORY_SIZE && addr2 < MEMORY_SIZE) { char* res = strstr((char*)&memory[addr1], (char*)&memory[addr2]); registers[reg1] = (double)(res ? res - (char*)&memory[addr1] : -1); }
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        else if (opcode == OP_STR_ATOI_REG_MEM) {
            reg1 = decode_register(); address = decode_address();
            if (reg1 != REG_INVALID && address < MEMORY_SIZE) registers[reg1] = (double)atoi((char*)&memory[address]);
            set_zero_flag_float(registers[reg1]);
            set_sign_flag_float(registers[reg1]);
        }
        else if (opcode == OP_STR_ITOA_MEM_REG_REG) {
            address = decode_address(); reg1 = decode_register(); reg2 = decode_register();
            if (address < MEMORY_SIZE && reg1 != REG_INVALID && reg2 != REG_INVALID) {
                sprintf((char*)&memory[address], "%d", (int)registers[reg1]);
            }
        }
        else if (opcode == OP_STR_SUBSTR_MEM_MEM_REG_REG) {
            uint32_t dest_addr = decode_address(); uint32_t src_addr = decode_address(); reg1 = decode_register(); reg2 = decode_register();
            if (dest_addr < MEMORY_SIZE && src_addr < MEMORY_SIZE && reg1 != REG_INVALID && reg2 != REG_INVALID) {
                char* src = (char*)&memory[src_addr];
                char* dest = (char*)&memory[dest_addr];
                int start = (int)registers[reg1];
                int len = (int)registers[reg2];
                int src_len = strlen(src);
                if (start >= 0 && start < src_len && len > 0) {
                    strncpy(dest, src + start, len);
                    dest[len] = '\0';
                }
                else if (len <= 0) {
                    dest[0] = '\0';
                }
                else {
                    dest[0] = '\0';
                }
            }
        }
        else if (opcode == OP_STR_FMT_MEM_MEM_REG_REG) {
            uint32_t dest_addr = decode_address();
            uint32_t fmt_addr = decode_address();
            reg1 = decode_register();
            reg2 = decode_register();
            if (dest_addr < MEMORY_SIZE && fmt_addr < MEMORY_SIZE && reg1 != REG_INVALID && reg2 != REG_INVALID) {
                sprintf((char*)&memory[dest_addr], (char*)&memory[fmt_addr], registers[reg1], registers[reg2]);
            }
        }
        break;
    }

    // Memory Standard Library Implementation
    case OP_MEM_CPY_MEM_MEM_REG: case OP_MEM_SET_MEM_REG_VAL: case OP_MEM_FREE_MEM: case OP_MEM_SET_MEM_REG_REG: {
        if (opcode == OP_MEM_CPY_MEM_MEM_REG) {
            uint32_t dest_addr = decode_address(); uint32_t src_addr = decode_address(); reg1 = decode_register();
            if (dest_addr < MEMORY_SIZE && src_addr < MEMORY_SIZE && reg1 != REG_INVALID) memcpy(&memory[dest_addr], &memory[src_addr], (uint32_t)registers[reg1]);
        }
        else if (opcode == OP_MEM_SET_MEM_REG_VAL) {
            uint32_t dest_addr = decode_address(); reg1 = decode_register(); value_uint32 = decode_value_uint32();
            if (dest_addr < MEMORY_SIZE && reg1 != REG_INVALID) memset(&memory[dest_addr], (uint8_t)registers[reg1], value_uint32);
        }
        else if (opcode == OP_MEM_SET_MEM_REG_REG) {
            uint32_t dest_addr = decode_address();
            reg1 = decode_register();
            reg2 = decode_register();
            if (dest_addr < MEMORY_SIZE && reg1 != REG_INVALID && reg2 != REG_INVALID) {
                memset(&memory[dest_addr], (uint8_t)registers[reg1], (uint32_t)registers[reg2]);
            }
        }
        else if (opcode == OP_MEM_FREE_MEM) {
            address = decode_address();
            if (address < MEMORY_SIZE) {
                uint32_t buffer_size = 0;
                for (int i = 0; i < buffer_count; i++) { if (buffers[i].address == address) { buffer_size = buffers[i].size; break; } }
                if (buffer_size > 0) memset(&memory[address], 0, buffer_size);
                else printf("Warning: mem.clear called on address without known buffer size.\n");
            }
            else printf("Error: MEMFREE address out of bounds!\n");
        }
        break;
    }

                               // System Library Opcodes Implementation
    case OP_SYS_PRINT_CHAR: { reg1 = decode_register(); if (reg1 != REG_INVALID) sys_print_char((char)(uint32_t)registers[reg1]); cursor_x++; break; }
    case OP_SYS_CLEAR_SCREEN: sys_clear_screen(); break;
    case OP_SYS_PRINT_STRING: { reg1 = decode_register(); if (reg1 != REG_INVALID) sys_print_string((uint32_t)registers[reg1]); break; }
    case OP_SYS_PRINT_NEWLINE: sys_print_newline(); break;
    case OP_SYS_SET_CURSOR_POS: { reg1 = decode_register(); reg2 = decode_register(); if (reg1 != REG_INVALID && reg2 != REG_INVALID) sys_set_cursor_pos((uint32_t)registers[reg1], (uint32_t)registers[reg2]); break; }
    case OP_SYS_GET_CURSOR_POS: { reg1 = decode_register(); reg2 = decode_register(); uint32_t x_pos, y_pos; sys_get_cursor_pos(&x_pos, &y_pos); if (reg1 != REG_INVALID && reg2 != REG_INVALID) { registers[reg1] = (double)x_pos; registers[reg2] = (double)y_pos; } break; }
    case OP_SYS_SET_TEXT_COLOR: { reg1 = decode_register(); if (reg1 != REG_INVALID) sys_set_text_color((uint32_t)registers[reg1]); break; }
    case OP_SYS_RESET_TEXT_COLOR: sys_reset_text_color(); break;
    case OP_SYS_PRINT_NUMBER_DEC: { reg1 = decode_register(); if (reg1 != REG_INVALID) sys_print_number_dec(registers[reg1]); break; }
    case OP_SYS_PRINT_NUMBER_HEX: { reg1 = decode_register(); if (reg1 != REG_INVALID) sys_print_number_hex((uint32_t)registers[reg1]); break; }
    case OP_SYS_NUMBER_TO_STRING: { reg1 = decode_register(); reg2 = decode_register(); reg3 = decode_register(); if (reg1 != REG_INVALID && reg2 != REG_INVALID && reg3 != REG_INVALID) sys_number_to_string((uint32_t)registers[reg1], (uint32_t)registers[reg2], (uint32_t)registers[reg3]); break; }
    case OP_SYS_READ_CHAR: { reg1 = decode_register(); if (reg1 != REG_INVALID) registers[reg1] = (double)sys_read_char(); break; }
    case OP_SYS_READ_STRING: { reg1 = decode_register(); reg2 = decode_register(); if (reg1 != REG_INVALID && reg2 != REG_INVALID) sys_read_string((uint32_t)registers[reg1], (uint32_t)registers[reg2]); break; }
    case OP_SYS_GET_KEY_PRESS: { reg1 = decode_register(); if (reg1 != REG_INVALID) registers[reg1] = (double)sys_get_key_press(); break; }
    case OP_SYS_GET_CPU_VER: { reg1 = decode_register(); if (reg1 != REG_INVALID) registers[reg1] = sys_get_cpu_ver(); break; }
    case OP_SYS_WAIT: { reg1 = decode_register(); if (reg1 != REG_INVALID) sys_wait((uint32_t)registers[reg1]); break; }
    case OP_SYS_TIME_REG: { reg1 = decode_register(); if (reg1 != REG_INVALID) registers[reg1] = sys_time(); break; }

                        // Disk Standard Library Implementation
    case OP_DISK_GET_SIZE_REG: {
        reg1 = decode_register();
        uint32_t disk_size;
        DiskResultCode result = disk_get_size(&disk_size);
        if (result == DISK_OK && reg1 != REG_INVALID) {
            registers[reg1] = (double)disk_size;
        }
        else {
            printf("DISK Error: Get Size failed with code %d\n", result);
            registers[reg1] = (double)result;
        }
        break;
    }
    case OP_DISK_READ_SECTOR_MEM_REG_REG: {
        uint32_t address_mem;
        reg1 = decode_register();
        reg2 = decode_register();
        address_mem = decode_address();

        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            DiskResultCode result = disk_read_sector((uint32_t)registers[reg1], address_mem, (uint32_t)registers[reg2]);
            if (result != DISK_OK) {
                printf("DISK Error: Read Sector failed with code %d\n", result);
            }
        }
        break;
    }
    case OP_DISK_WRITE_SECTOR_MEM_REG_REG: {
        uint32_t address_mem;
        reg1 = decode_register();
        reg2 = decode_register();
        address_mem = decode_address();

        if (reg1 != REG_INVALID && reg2 != REG_INVALID) {
            DiskResultCode result = disk_write_sector((uint32_t)registers[reg1], address_mem, (uint32_t)registers[reg2]);
            if (result != DISK_OK) {
                printf("DISK Error: Write Sector failed with code %d\n", result);
            }
        }
        break;
    }
    case OP_DISK_CREATE_IMAGE: {
        DiskResultCode result = create_disk_image();
        if (result != DISK_OK) {
            printf("DISK Error: Create Image failed with code %d\n", result);
        }
        break;
    }
    case OP_DISK_GET_VOLUME_LABEL_MEM: {
        address = decode_address();
        DiskResultCode result = disk_get_volume_label(address);
        if (result != DISK_OK) {
            printf("DISK Error: Get Volume Label failed with code %d\n", result);
        }
        break;
    }
    case OP_DISK_SET_VOLUME_LABEL_MEM: {
        address = decode_address();
        DiskResultCode result = disk_set_volume_label(address);
        if (result != DISK_OK) {
            printf("DISK Error: Set Volume Label failed with code %d\n", result);
        }
        break;
    }


    case OP_INVALID: printf("Invalid Opcode!\n"); running = false; break;
    default: printf("Unknown Opcode: %d\n", opcode); running = false; break;
    }
}

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

bool is_register_str(const char* str) {
    return register_from_string(str) != REG_INVALID;
}

bool is_memory_address_str(const char* str) {
    return (str[0] == '[' && str[strlen(str) - 1] == ']');
}

Opcode opcode_from_string(const char* op_str, char* operand1, char* operand2, char* operand3, char* operand4) {
    if (strcmp(op_str, "NOP") == 0) return OP_NOP;
    if (strcmp(op_str, "MOV") == 0) {
        if (operand1 && operand2) {
            if (is_register_str(operand1)) {
                if (is_register_str(operand2)) return OP_MOV_REG_REG;
                else if (is_memory_address_str(operand2)) return OP_MOV_REG_MEM;
                else if (get_label_address(operand2) != -1) return OP_LEA_REG_MEM;
                else return OP_MOV_REG_VAL;
            }
            else if (is_memory_address_str(operand1) && is_register_str(operand2)) {
                return OP_MOV_MEM_REG;
            }
        }
    }
    if (strcmp(op_str, "ADD") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_ADD_REG_REG; else return OP_ADD_REG_VAL; } }
    if (strcmp(op_str, "SUB") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_SUB_REG_REG; else return OP_SUB_REG_VAL; } }
    if (strcmp(op_str, "MUL") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_MUL_REG_REG; else return OP_MUL_REG_VAL; } }
    if (strcmp(op_str, "DIV") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_DIV_REG_REG; else return OP_DIV_REG_VAL; } }
    if (strcmp(op_str, "MOD") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_MOD_REG_REG; else return OP_MOD_REG_VAL; } }
    if (strcmp(op_str, "AND") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_AND_REG_REG; else return OP_AND_REG_VAL; } }
    if (strcmp(op_str, "OR") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_OR_REG_REG; else return OP_OR_REG_VAL; } }
    if (strcmp(op_str, "XOR") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_XOR_REG_REG; else return OP_XOR_REG_VAL; } }
    if (strcmp(op_str, "NOT") == 0) { if (operand1 && is_register_str(operand1)) return OP_NOT_REG; }
    if (strcmp(op_str, "NEG") == 0) { if (operand1 && is_register_str(operand1)) return OP_NEG_REG; }
    if (strcmp(op_str, "CMP") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_CMP_REG_REG; else return OP_CMP_REG_VAL; } }
    if (strcmp(op_str, "TEST") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_TEST_REG_REG; else return OP_TEST_REG_VAL; } }
    if (strcmp(op_str, "IMUL") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_IMUL_REG_REG; }
    if (strcmp(op_str, "IDIV") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_IDIV_REG_REG; }
    if (strcmp(op_str, "MOVZX") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_MOVZX_REG_REG; else if (is_memory_address_str(operand2)) return OP_MOVZX_REG_MEM; } }
    if (strcmp(op_str, "MOVSX") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_MOVSX_REG_REG; else if (is_memory_address_str(operand2)) return OP_MOVSX_REG_MEM; } }
    if (strcmp(op_str, "LEA") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_memory_address_str(operand2) || get_label_address(operand2) != -1) return OP_LEA_REG_MEM; } }
    if (strcmp(op_str, "JMP") == 0) { if (operand1) return OP_JMP; }
    if (strcmp(op_str, "JNZ") == 0 || strcmp(op_str, "JMP_NZ") == 0) { if (operand1) return OP_JMP_NZ; }
    if (strcmp(op_str, "JZ") == 0 || strcmp(op_str, "JMP_Z") == 0) { if (operand1) return OP_JMP_Z; }
    if (strcmp(op_str, "JS") == 0 || strcmp(op_str, "JMP_S") == 0) { if (operand1) return OP_JMP_S; }
    if (strcmp(op_str, "JNS") == 0 || strcmp(op_str, "JMP_NS") == 0) { if (operand1) return OP_JMP_NS; }
    if (strcmp(op_str, "JC") == 0 || strcmp(op_str, "JMP_C") == 0) { if (operand1) return OP_JMP_C; }
    if (strcmp(op_str, "JNC") == 0 || strcmp(op_str, "JMP_NC") == 0) { if (operand1) return OP_JMP_NC; }
    if (strcmp(op_str, "JO") == 0 || strcmp(op_str, "JMP_O") == 0) { if (operand1) return OP_JMP_O; }
    if (strcmp(op_str, "JNO") == 0 || strcmp(op_str, "JMP_NO") == 0) { if (operand1) return OP_JMP_NO; }
    if (strcmp(op_str, "JGE") == 0 || strcmp(op_str, "JMP_GE") == 0) { if (operand1) return OP_JMP_GE; }
    if (strcmp(op_str, "JLE") == 0 || strcmp(op_str, "JMP_LE") == 0) { if (operand1) return OP_JMP_LE; }
    if (strcmp(op_str, "JG") == 0 || strcmp(op_str, "JMP_G") == 0) { if (operand1) return OP_JMP_G; }
    if (strcmp(op_str, "JL") == 0 || strcmp(op_str, "JMP_L") == 0) { if (operand1) return OP_JMP_L; }
    if (strcmp(op_str, "HLT") == 0) return OP_HLT;
    if (strcmp(op_str, "INC") == 0) { if (operand1 && is_register_str(operand1)) return OP_INC_REG; else if (is_memory_address_str(operand1)) return OP_INC_MEM; }
    if (strcmp(op_str, "DEC") == 0) { if (operand1 && is_register_str(operand1)) return OP_DEC_REG; else if (is_memory_address_str(operand1)) return OP_DEC_MEM; }
    if (strcmp(op_str, "SHL") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_SHL_REG_REG; else return OP_SHL_REG_VAL; } }
    if (strcmp(op_str, "SHR") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_SHR_REG_REG; else return OP_SHR_REG_VAL; } }
    if (strcmp(op_str, "SAR") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_SAR_REG_REG; else return OP_SAR_REG_VAL; } }
    if (strcmp(op_str, "ROL") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_ROL_REG_REG; else return OP_ROL_REG_VAL; } }
    if (strcmp(op_str, "ROR") == 0) { if (operand1 && operand2 && is_register_str(operand1)) { if (is_register_str(operand2)) return OP_ROR_REG_REG; else return OP_ROR_REG_VAL; } }
    if (strcmp(op_str, "RND") == 0) { if (operand1 && is_register_str(operand1)) return OP_RND_REG; }
    if (strcmp(op_str, "PUSH") == 0) { if (operand1 && is_register_str(operand1)) return OP_PUSH_REG; }
    if (strcmp(op_str, "POP") == 0) { if (operand1 && is_register_str(operand1)) return OP_POP_REG; }
    if (strcmp(op_str, "CALL") == 0) { if (operand1) return OP_CALL_ADDR; }
    if (strcmp(op_str, "RET") == 0) return OP_RET;
    if (strcmp(op_str, "XCHG") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_XCHG_REG_REG; }
    if (strcmp(op_str, "BSWAP") == 0) { if (operand1 && is_register_str(operand1)) return OP_BSWAP_REG; }
    if (strcmp(op_str, "SETZ") == 0) { if (operand1 && is_register_str(operand1)) return OP_SETZ_REG; }
    if (strcmp(op_str, "SETNZ") == 0) { if (operand1 && is_register_str(operand1)) return OP_SETNZ_REG; }


    if (strncmp(op_str, "math.", 5) == 0) {
        char* math_func = op_str + 5;
        if (strcmp(math_func, "add") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_ADD; }
        else if (strcmp(math_func, "sub") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_SUB; }
        else if (strcmp(math_func, "mul") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MUL; }
        else if (strcmp(math_func, "div") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_DIV; }
        else if (strcmp(math_func, "mod") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MOD; }
        else if (strcmp(math_func, "abs") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_ABS; }
        else if (strcmp(math_func, "sin") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_SIN; }
        else if (strcmp(math_func, "cos") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_COS; }
        else if (strcmp(math_func, "tan") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_TAN; }
        else if (strcmp(math_func, "asin") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_ASIN; }
        else if (strcmp(math_func, "acos") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_ACOS; }
        else if (strcmp(math_func, "atan") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_ATAN; }
        else if (strcmp(math_func, "pow") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_POW; }
        else if (strcmp(math_func, "sqrt") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_SQRT; }
        else if (strcmp(math_func, "log") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_LOG; }
        else if (strcmp(math_func, "exp") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_EXP; }
        else if (strcmp(math_func, "floor") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_FLOOR; }
        else if (strcmp(math_func, "ceil") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_CEIL; }
        else if (strcmp(math_func, "round") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_ROUND; }
        else if (strcmp(math_func, "min") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MIN; }
        else if (strcmp(math_func, "max") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_MAX; }
        else if (strcmp(math_func, "neg") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_NEG; }
        else if (strcmp(math_func, "atan2") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_MATH_ATAN2; }
        else if (strcmp(math_func, "log10") == 0) { if (operand1 && is_register_str(operand1)) return OP_MATH_LOG10; }
        else if (strcmp(math_func, "clamp") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && is_register_str(operand2) && is_register_str(operand3)) return OP_MATH_CLAMP; }
        else if (strcmp(math_func, "lerp") == 0) { if (operand1 && operand2 && operand3 && operand4 && is_register_str(operand1) && is_register_str(operand2) && is_register_str(operand3) && is_register_str(operand4)) return OP_MATH_LERP; }
    }
    else if (strncmp(op_str, "str.", 4) == 0) {
        char* str_func = op_str + 4;
        if (strcmp(str_func, "len") == 0) { if (operand1 && operand2 && is_register_str(operand1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) return OP_STR_LEN_REG_MEM; }
        else if (strcmp(str_func, "cpy") == 0) { if (operand1 && operand2 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) return OP_STR_CPY_MEM_MEM; }
        else if (strcmp(str_func, "cat") == 0) { if (operand1 && operand2 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) return OP_STR_CAT_MEM_MEM; }
        else if (strcmp(str_func, "cmp") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && (is_memory_address_str(operand3) || get_label_address(operand3) != -1)) return OP_STR_CMP_REG_MEM_MEM; }
        else if (strcmp(str_func, "ncpy") == 0) { if (operand1 && operand2 && operand3 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && is_register_str(operand3)) return OP_STR_NCPY_MEM_MEM_REG; }
        else if (strcmp(str_func, "ncat") == 0) { if (operand1 && operand2 && operand3 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && is_register_str(operand3)) return OP_STR_NCAT_MEM_MEM_REG; }
        else if (strcmp(str_func, "toupper") == 0) { if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1)) return OP_STR_TOUPPER_MEM; }
        else if (strcmp(str_func, "tolower") == 0) { if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1)) return OP_STR_TOLOWER_MEM; }
        else if (strcmp(str_func, "chr") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && !is_register_str(operand3) && !is_memory_address_str(operand3)) return OP_STR_CHR_REG_MEM_VAL; }
        else if (strcmp(str_func, "str") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && (is_memory_address_str(operand3) || get_label_address(operand3) != -1)) return OP_STR_STR_REG_MEM_MEM; }
        else if (strcmp(str_func, "atoi") == 0) { if (operand1 && operand2 && is_register_str(operand1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1)) return OP_STR_ATOI_REG_MEM; }
        else if (strcmp(str_func, "itoa") == 0) { if (operand1 && operand2 && operand3 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && is_register_str(operand2) && is_register_str(operand3)) return OP_STR_ITOA_MEM_REG_REG; }
        else if (strcmp(str_func, "substr") == 0) { if (operand1 && operand2 && operand3 && operand4 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && is_register_str(operand3) && is_register_str(operand4)) return OP_STR_SUBSTR_MEM_MEM_REG_REG; }
        else if (strcmp(str_func, "fmt") == 0) { if (operand1 && operand2 && operand3 && operand4 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && is_register_str(operand3) && is_register_str(operand4)) return OP_STR_FMT_MEM_MEM_REG_REG; }
    }
    else if (strncmp(op_str, "mem.", 4) == 0) {
        char* mem_func = op_str + 4;
        if (strcmp(mem_func, "cpy") == 0) { if (operand1 && operand2 && operand3 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1) && (is_memory_address_str(operand2) || get_label_address(operand2) != -1) && is_register_str(operand3)) return OP_MEM_CPY_MEM_MEM_REG; }
        else if (strcmp(mem_func, "set") == 0) {
            if (operand1 && operand2 && operand3) {
                if ((is_memory_address_str(operand1) || get_label_address(operand1) != -1) && is_register_str(operand2) && !is_register_str(operand3) && !is_memory_address_str(operand3)) return OP_MEM_SET_MEM_REG_VAL;
                else if ((is_memory_address_str(operand1) || get_label_address(operand1) != -1) && is_register_str(operand2) && is_register_str(operand3)) return OP_MEM_SET_MEM_REG_REG;
            }
        }
        else if (strcmp(mem_func, "clear") == 0) { if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1)) return OP_MEM_FREE_MEM; }
    }
    else if (strncmp(op_str, "sys.", 4) == 0) {
        char* sys_func = op_str + 4;
        if (strcmp(sys_func, "print_char") == 0) { if (operand1 && (is_register_str(operand1) || !is_register_str(operand1) && !is_memory_address_str(operand1))) return OP_SYS_PRINT_CHAR; }
        else if (strcmp(sys_func, "clear_screen") == 0) return OP_SYS_CLEAR_SCREEN;
        else if (strcmp(sys_func, "print_string") == 0) { if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1 || is_register_str(operand1))) return OP_SYS_PRINT_STRING; }
        else if (strcmp(sys_func, "newline") == 0) return OP_SYS_PRINT_NEWLINE;
        else if (strcmp(sys_func, "set_cursor_pos") == 0) { if (operand1 && operand2 && (is_register_str(operand1) || !is_register_str(operand1) && !is_memory_address_str(operand1)) && (is_register_str(operand2) || !is_register_str(operand2) && !is_memory_address_str(operand2))) return OP_SYS_SET_CURSOR_POS; }
        else if (strcmp(sys_func, "get_cursor_pos") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_SYS_GET_CURSOR_POS; }
        else if (strcmp(sys_func, "set_text_color") == 0) { if (operand1 && (is_register_str(operand1) || !is_register_str(operand1) && !is_memory_address_str(operand1))) return OP_SYS_SET_TEXT_COLOR; }
        else if (strcmp(sys_func, "reset_text_color") == 0) return OP_SYS_RESET_TEXT_COLOR;
        else if (strcmp(sys_func, "print_number_dec") == 0) { if (operand1 && is_register_str(operand1)) return OP_SYS_PRINT_NUMBER_DEC; }
        else if (strcmp(sys_func, "print_number_hex") == 0) { if (operand1 && is_register_str(operand1)) return OP_SYS_PRINT_NUMBER_HEX; }
        else if (strcmp(sys_func, "number_to_string") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && is_register_str(operand2) && is_register_str(operand3)) return OP_SYS_NUMBER_TO_STRING; }
        else if (strcmp(sys_func, "read_char") == 0) { if (operand1 && is_register_str(operand1)) return OP_SYS_READ_CHAR; }
        else if (strcmp(sys_func, "read_string") == 0) { if (operand1 && operand2 && is_register_str(operand1) && is_register_str(operand2)) return OP_SYS_READ_STRING; }
        else if (strcmp(sys_func, "get_key_press") == 0) { if (operand1 && is_register_str(operand1)) return OP_SYS_GET_KEY_PRESS; }
        else if (strcmp(sys_func, "cpu_ver") == 0) { if (operand1 && is_register_str(operand1)) return OP_SYS_GET_CPU_VER; }
        else if (strcmp(sys_func, "wait") == 0) { if (operand1 && (is_register_str(operand1) || !is_register_str(operand1) && !is_memory_address_str(operand1))) return OP_SYS_WAIT; }
        else if (strcmp(sys_func, "time") == 0) { if (operand1 && is_register_str(operand1)) return OP_SYS_TIME_REG; }
    }
    else if (strncmp(op_str, "disk.", 5) == 0) {
        char* disk_func = op_str + 5;
        if (strcmp(disk_func, "get_size") == 0) { if (operand1 && is_register_str(operand1)) return OP_DISK_GET_SIZE_REG; }
        else if (strcmp(disk_func, "read_sector") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && is_register_str(operand2) && (is_memory_address_str(operand3) || get_label_address(operand3) != -1)) return OP_DISK_READ_SECTOR_MEM_REG_REG; }
        else if (strcmp(disk_func, "write_sector") == 0) { if (operand1 && operand2 && operand3 && is_register_str(operand1) && is_register_str(operand2) && (is_memory_address_str(operand3) || get_label_address(operand3) != -1)) return OP_DISK_WRITE_SECTOR_MEM_REG_REG; }
        else if (strcmp(disk_func, "create_image") == 0) return OP_DISK_CREATE_IMAGE;
        else if (strcmp(disk_func, "get_volume_label") == 0) { if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1)) return OP_DISK_GET_VOLUME_LABEL_MEM; }
        else if (strcmp(disk_func, "set_volume_label") == 0) { if (operand1 && (is_memory_address_str(operand1) || get_label_address(operand1) != -1)) return OP_DISK_SET_VOLUME_LABEL_MEM; }
    }

    return OP_INVALID;
}

RegisterIndex register_from_string(const char* reg_str) {
    if (!reg_str) return REG_INVALID;

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
    for (int i = 0; i < buffer_count; i++) {
        if (strcmp(buffers[i].name, label_name) == 0) {
            return buffers[i].address;
        }
    }
    return -1;
}

double parse_value_double(const char* value_str) {
    if (!value_str) return 0.0;

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

uint32_t parse_address(const char* addr_str) {
    char temp_addr_str[64];
    if (!addr_str) return 0;
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

typedef enum {
    PREPROCESSOR_STATE_NORMAL,
    PREPROCESSOR_STATE_IFDEF_FALSE
} PreprocessorState;

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
    char lst_filename[256];
    strcpy(lst_filename, rom_filename);
    strcat(lst_filename, ".lst");
    FILE* lst_file = fopen(lst_filename, "w");
    if (!lst_file) {
        perror("Error opening LST file for writing");
        fclose(asm_file);
        fclose(rom_file);
        return -1;
    }

    fprintf(lst_file, "Assembly Listing for: %s\n\n", asm_filename);
    fprintf(lst_file, "Line No. | Address  | Assembly Code                  | Binary Code         | Comment\n");
    fprintf(lst_file, "---------|----------|--------------------------------|---------------------|---------\n");


    memset(memory, 0, MEMORY_SIZE);
    program_counter = 0;
    macro_count = 0;
    label_count = 0;
    string_count = 0;
    buffer_count = 0;
    data_section_start = 0;
    uint32_t rom_offset = 0;

    char line[256];
    int line_number = 1;
    PreprocessorState preprocessor_state[MAX_PREPROCESSOR_DEPTH];
    int preprocessor_depth = 0;
    preprocessor_state[0] = PREPROCESSOR_STATE_NORMAL;

    rewind(asm_file);
    program_counter = 0;
    while (fgets(line, sizeof(line), asm_file)) {
        char original_line[256];
        strcpy(original_line, line);
        char* line_ptr = line;
        char* token = strtok(line_ptr, " ,\t\n");
        if (!token || token[0] == ';') {
            line_number++;
            continue;
        }

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
            char* offset_str = strtok(NULL, " ,\t\n");
            if (offset_str) {
                rom_offset = parse_address(offset_str);
                if (rom_offset > MEMORY_SIZE) {
                    fprintf(stderr, "Error: Offset too large on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    fclose(lst_file);
                    return -1;
                }
                if (rom_offset % 1 != 0) {
                    fprintf(stderr, "Error: Offset must be byte aligned on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    fclose(lst_file);
                    return -1;
                }

            }
            else {
                fprintf(stderr, "Error: Missing offset value in #offset directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
                return -1;
            }
            line_number++;
            continue;
        }
        else if (strcmp(token, ".STRING") == 0) {
            char* string_name = strtok(NULL, " ,\t\n");
            char* string_value_token = strtok(NULL, "\n");

            if (!string_name) {
                fprintf(stderr, "Error: Missing string name in .STRING directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
                return -1;
            }
            if (!string_value_token) {
                fprintf(stderr, "Error: Missing string value in .STRING directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
                return -1;
            }

            const char* macro_value = get_macro_value(string_value_token);
            const char* string_value_with_quotes;
            if (macro_value != NULL) {
                string_value_with_quotes = macro_value;
            }
            else {
                string_value_with_quotes = string_value_token;
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
                        fclose(lst_file);
                        return -1;
                    }
                }
                else {
                    if (macro_value == NULL) {
                        fprintf(stderr, "Error: Invalid string syntax on line %d. String must be enclosed in single quotes.\n", line_number);
                        fclose(asm_file);
                        fclose(rom_file);
                        fclose(lst_file);
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
                fclose(lst_file);
                return -1;
            }
            line_number++;
            continue;
        }
        else if (strcmp(token, ".BUFFER") == 0) {
            char* buffer_name = strtok(NULL, " ,\t\n");
            char* buffer_size_str = strtok(NULL, " ,\t\n");

            if (!buffer_name) {
                fprintf(stderr, "Error: Missing buffer name in .BUFFER directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
                return -1;
            }
            if (!buffer_size_str) {
                fprintf(stderr, "Error: Missing buffer size in .BUFFER directive on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
                return -1;
            }

            uint32_t buffer_size = parse_address(buffer_size_str);
            if (buffer_size == 0 || buffer_size > MEMORY_SIZE) {
                fprintf(stderr, "Error: Invalid buffer size '%u' on line %d.\n", buffer_size, line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
                return -1;
            }

            if (buffer_count < MAX_BUFFERS) {
                strncpy(buffers[buffer_count].name, buffer_name, sizeof(buffers[buffer_count].name) - 1);
                buffers[buffer_count].name[sizeof(buffers[buffer_count].name) - 1] = '\0';
                buffers[buffer_count].size = buffer_size;
                buffer_count++;
                program_counter += buffer_size;
            }
            else {
                fprintf(stderr, "Error: Buffer limit reached on line %d.\n", line_number);
                fclose(asm_file);
                fclose(rom_file);
                fclose(lst_file);
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
                        fclose(lst_file);
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "Error: Invalid #define syntax on line %d. Expected #define MACRO_NAME VALUE\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    fclose(lst_file);
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
                        fclose(lst_file);
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "Error: Max preprocessor nesting depth reached on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    fclose(lst_file);
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
                        fclose(lst_file);
                        return -1;
                    }
                }
                else {
                    fprintf(stderr, "Error: Max preprocessor nesting depth reached on line %d.\n", line_number);
                    fclose(asm_file);
                    fclose(rom_file);
                    fclose(lst_file);
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
                    fclose(lst_file);
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
                    fclose(lst_file);
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
                fclose(lst_file);
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
                fclose(lst_file);
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
                fclose(lst_file);
                return -1;
            }
            line_number++;
            continue;
        }

        char* operand1_str = strtok(NULL, " ,\t\n");
        char* operand2_str = strtok(NULL, " ,\t\n");
        char* operand3_str = strtok(NULL, " ,\t\n");
        char* operand4_str = strtok(NULL, " ,\t\n");
        Opcode opcode = opcode_from_string(token, operand1_str, operand2_str, operand3_str, operand4_str);

        if (opcode == OP_INVALID) {
            fprintf(stderr, "Error: Invalid opcode '%s' on line %d.\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            fclose(lst_file);
            return -1;
        }

        memory[program_counter++] = (uint8_t)opcode;

        int instruction_bytes = 1;
        switch (opcode) {
        case OP_MOV_REG_VAL:
        case OP_ADD_REG_VAL:
        case OP_SUB_REG_VAL:
        case OP_MUL_REG_VAL:
        case OP_DIV_REG_VAL:
        case OP_MOD_REG_VAL:
        case OP_CMP_REG_VAL:
        case OP_TEST_REG_VAL:
        case OP_AND_REG_VAL:
        case OP_OR_REG_VAL:
        case OP_XOR_REG_VAL:
            instruction_bytes += 9; break;
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
        case OP_XCHG_REG_REG:
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
            instruction_bytes += 2; break;
        case OP_MOV_REG_MEM:
        case OP_MOV_MEM_REG:
        case OP_LEA_REG_MEM:
        case OP_MOVZX_REG_MEM:
        case OP_MOVSX_REG_MEM:
        case OP_STR_LEN_REG_MEM:
        case OP_STR_ATOI_REG_MEM:
        case OP_DISK_GET_VOLUME_LABEL_MEM:
        case OP_DISK_SET_VOLUME_LABEL_MEM:
            instruction_bytes += 5; break;
        case OP_NOT_REG:
        case OP_NEG_REG:
        case OP_INC_REG:
        case OP_DEC_REG:
        case OP_RND_REG:
        case OP_PUSH_REG:
        case OP_POP_REG:
        case OP_BSWAP_REG:
        case OP_SETZ_REG:
        case OP_SETNZ_REG:
        case OP_SYS_PRINT_CHAR:
        case OP_SYS_PRINT_STRING:
        case OP_SYS_SET_TEXT_COLOR:
        case OP_SYS_PRINT_NUMBER_DEC:
        case OP_SYS_PRINT_NUMBER_HEX:
        case OP_SYS_READ_CHAR:
        case OP_SYS_GET_KEY_PRESS:
        case OP_SYS_GET_CPU_VER:
        case OP_SYS_WAIT:
        case OP_SYS_TIME_REG:
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
        case OP_DISK_GET_SIZE_REG:
            instruction_bytes += 5; break;
        case OP_MATH_CLAMP:
            instruction_bytes += 3; break;
        case OP_MATH_LERP:
            instruction_bytes += 4; break;
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
        case OP_DISK_CREATE_IMAGE:
            instruction_bytes += 4; break;
        case OP_INC_MEM:
        case OP_DEC_MEM:
        case OP_STR_CPY_MEM_MEM:
        case OP_STR_CAT_MEM_MEM:
            instruction_bytes += 8; break;
        case OP_STR_CMP_REG_MEM_MEM:
            instruction_bytes += 9; break;
        case OP_STR_NCPY_MEM_MEM_REG:
        case OP_STR_NCAT_MEM_MEM_REG:
        case OP_DISK_READ_SECTOR_MEM_REG_REG:
        case OP_DISK_WRITE_SECTOR_MEM_REG_REG:
            instruction_bytes += 9; break;
        case OP_MEM_CPY_MEM_MEM_REG:
            instruction_bytes += 9; break;
        case OP_STR_ITOA_MEM_REG_REG:
        case OP_STR_FMT_MEM_MEM_REG_REG:
            instruction_bytes += 9; break;
        case OP_MEM_SET_MEM_REG_VAL:
            instruction_bytes += 10; break;
        case OP_MEM_SET_MEM_REG_REG:
            instruction_bytes += 6; break;
        case OP_MEM_FREE_MEM:
            instruction_bytes += 5; break;
        case OP_STR_TOUPPER_MEM:
        case OP_STR_TOLOWER_MEM:
            instruction_bytes += 5; break;
        case OP_STR_CHR_REG_MEM_VAL:
            instruction_bytes += 10; break;
        case OP_STR_STR_REG_MEM_MEM:
            instruction_bytes += 9; break;
        case OP_SYS_SET_CURSOR_POS:
        case OP_SYS_NUMBER_TO_STRING:
        case OP_SYS_READ_STRING:
        case OP_SYS_GET_CURSOR_POS:
            instruction_bytes += 3; break;
        case OP_SYS_RESET_TEXT_COLOR:
        case OP_SYS_PRINT_NEWLINE:
        case OP_SYS_CLEAR_SCREEN:
        case OP_NOP:
        case OP_HLT:
        case OP_RET:
        case OP_DISK_FORMAT_DISK:
            instruction_bytes = 1; break;
        default:
            fprintf(stderr, "Assembler Error (Pass 1): Unhandled opcode size calculation for '%s' on line %d.\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            fclose(lst_file);
            return -1;
        }
        program_counter += instruction_bytes - 1;

        line_number++;
    }
    if (preprocessor_depth != 0) {
        fprintf(stderr, "Error: Unclosed #ifdef or #ifndef block.\n");
        fclose(asm_file);
        fclose(rom_file);
        fclose(lst_file);
        return -1;
    }

    data_section_start = program_counter;

    rewind(asm_file);
    program_counter = rom_offset;
    line_number = 1;
    uint32_t data_pointer = data_section_start;
    preprocessor_depth = 0;
    preprocessor_state[0] = PREPROCESSOR_STATE_NORMAL;

    for (uint32_t i = 0; i < rom_offset; ++i) {
        fputc(0x00, rom_file);
    }

    uint32_t current_address = rom_offset;

    while (fgets(line, sizeof(line), asm_file)) {
        char original_line[256];
        strcpy(original_line, line);

        char* line_ptr = line;
        char* token = strtok(line_ptr, " ,\t\n");
        if (!token || token[0] == ';') {
            fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, current_address, original_line, "", ";Comment or Empty Line\n");
            line_number++;
            continue;
        }

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
            fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, current_address, original_line, "", ";Preprocessor Directive\n");
            line_number++;
            continue;
        }


        if (strcmp(token, "#offset") == 0) {
            fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, current_address, original_line, "", ";Offset Directive\n");
            line_number++;
            continue;
        }
        if (strcmp(token, ".STRING") == 0) {
            char* string_name = strtok(NULL, " ,\t\n");
            char* string_value_token = strtok(NULL, "\n");
            const char* macro_value = get_macro_value(string_value_token);
            const char* string_value_with_quotes = macro_value != NULL ? macro_value : string_value_token;

            for (int i = 0; i < string_count; i++) {
                if (strcmp(strings[i].name, string_name) == 0) {
                    strings[i].address = data_pointer;
                    strcpy((char*)&memory[data_pointer], strings[i].value);
                    data_pointer += strlen(strings[i].value) + 1;
                    fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, strings[i].address + rom_offset, original_line, "", "; String Definition\n");
                    break;
                }
            }
            line_number++;
            current_address = data_pointer + rom_offset;
            continue;
        }
        if (strcmp(token, ".BUFFER") == 0) {
            char* buffer_name = strtok(NULL, " ,\t\n");

            for (int i = 0; i < buffer_count; i++) {
                if (strcmp(buffers[i].name, buffer_name) == 0) {
                    buffers[i].address = data_pointer;
                    data_pointer += buffers[i].size;
                    fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, buffers[i].address + rom_offset, original_line, "", "; Buffer Definition\n");
                    break;
                }
            }
            line_number++;
            current_address = data_pointer + rom_offset;
            continue;
        }
        if (token[0] == '#' || (strchr(token, ':') != NULL && strlen(token) > 1)) {
            fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, current_address, original_line, "", ";Label or Preprocessor\n");
            line_number++;
            continue;
        }

        char* reg1_str = strtok(NULL, " ,\t\n");
        char* reg2_str = strtok(NULL, " ,\t\n");
        char* reg3_str = strtok(NULL, " ,\t\n");
        char* reg4_str = strtok(NULL, " ,\t\n");
        Opcode opcode = opcode_from_string(token, reg1_str, reg2_str, reg3_str, reg4_str);

        if (opcode == OP_INVALID) {
            fprintf(stderr, "Error: Invalid opcode '%s' on line %d (Pass 2).\n", token, line_number);
            fclose(asm_file);
            fclose(rom_file);
            fclose(lst_file);
            return -1;
        }

        uint32_t instruction_start_address = program_counter;
        memory[program_counter++] = (uint8_t)opcode;
        char binary_output[256] = "";

        char mnemonic_output[256] = "";
        sprintf(mnemonic_output, "%s", token);
        if (reg1_str) sprintf(mnemonic_output + strlen(mnemonic_output), " %s", reg1_str);
        if (reg2_str) sprintf(mnemonic_output + strlen(mnemonic_output), ", %s", reg2_str);
        if (reg3_str) sprintf(mnemonic_output + strlen(mnemonic_output), ", %s", reg3_str);
        if (reg4_str) sprintf(mnemonic_output + strlen(mnemonic_output), ", %s", reg4_str);

        switch (opcode) {
        case OP_MOV_REG_VAL:
        case OP_ADD_REG_VAL:
        case OP_SUB_REG_VAL:
        case OP_MUL_REG_VAL:
        case OP_DIV_REG_VAL:
        case OP_MOD_REG_VAL:
        case OP_CMP_REG_VAL:
        case OP_TEST_REG_VAL:
        case OP_AND_REG_VAL:
        case OP_OR_REG_VAL:
        case OP_XOR_REG_VAL:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            double value = parse_value_double(reg2_str);
            memory[program_counter++] = (uint8_t)reg;
            *(double*)&memory[program_counter] = value;
            program_counter += 8;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            char val_hex[32] = "";
            for (int i = 0; i < 8; ++i) { sprintf(val_hex + i * 3, "%02X ", memory[instruction_start_address + 2 + i]); }
            strcat(binary_output, val_hex);

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
        case OP_XCHG_REG_REG:
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
            memory[program_counter++] = (uint8_t)reg1;
            memory[program_counter++] = (uint8_t)reg2;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg1_hex[8]; sprintf(reg1_hex, "%02X ", reg1); strcat(binary_output, reg1_hex);
            char reg2_hex[8]; sprintf(reg2_hex, "%02X ", reg2); strcat(binary_output, reg2_hex);

            break;
        }
        case OP_LEA_REG_MEM:
        case OP_MOV_REG_MEM:
        case OP_MOVZX_REG_MEM:
        case OP_MOVSX_REG_MEM:
        case OP_STR_LEN_REG_MEM:
        case OP_STR_ATOI_REG_MEM:
        case OP_INC_MEM:
        case OP_DEC_MEM:
        case OP_MEM_FREE_MEM:
        case OP_STR_TOUPPER_MEM:
        case OP_STR_TOLOWER_MEM:
        case OP_DISK_GET_VOLUME_LABEL_MEM:
        case OP_DISK_SET_VOLUME_LABEL_MEM:
        {
            RegisterIndex reg = REG_INVALID;
            if (opcode != OP_INC_MEM && opcode != OP_DEC_MEM && opcode != OP_MEM_FREE_MEM && opcode != OP_STR_TOUPPER_MEM && opcode != OP_STR_TOLOWER_MEM && opcode != OP_DISK_GET_VOLUME_LABEL_MEM && opcode != OP_DISK_SET_VOLUME_LABEL_MEM)
                reg = register_from_string(reg1_str);
            uint32_t address = parse_address((opcode == OP_INC_MEM || opcode == OP_DEC_MEM || opcode == OP_MEM_FREE_MEM || opcode == OP_STR_TOUPPER_MEM || opcode == OP_STR_TOLOWER_MEM || opcode == OP_DISK_GET_VOLUME_LABEL_MEM || opcode == OP_DISK_SET_VOLUME_LABEL_MEM) ? reg1_str : reg2_str);
            if (reg != REG_INVALID) memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            if (reg != REG_INVALID) { char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex); }
            char addr_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(addr_hex + i * 3, "%02X ", memory[instruction_start_address + (reg != REG_INVALID ? 2 : 1) + i]); }
            strcat(binary_output, addr_hex);
            break;
        }
        case OP_MOV_MEM_REG: {
            char* addr_str = reg1_str;
            RegisterIndex reg = register_from_string(reg2_str);
            uint32_t address = parse_address(addr_str);
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(addr_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); }
            strcat(binary_output, addr_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            break;
        }
        case OP_POP_REG: {
            RegisterIndex reg = register_from_string(reg1_str);
            memory[program_counter++] = (uint8_t)opcode;
            memory[program_counter++] = (uint8_t)reg;
            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            break;
        }
        case OP_NOT_REG:
        case OP_NEG_REG:
        case OP_INC_REG:
        case OP_DEC_REG:
        case OP_RND_REG:
        case OP_PUSH_REG:
        case OP_BSWAP_REG:
        case OP_SETZ_REG:
        case OP_SETNZ_REG:
        case OP_SYS_PRINT_CHAR:
        case OP_SYS_PRINT_STRING:
        case OP_SYS_SET_TEXT_COLOR:
        case OP_SYS_PRINT_NUMBER_DEC:
        case OP_SYS_PRINT_NUMBER_HEX:
        case OP_SYS_READ_CHAR:
        case OP_SYS_GET_KEY_PRESS:
        case OP_SYS_GET_CPU_VER:
        case OP_SYS_WAIT:
        case OP_SYS_TIME_REG:
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
        case OP_DISK_GET_SIZE_REG:
        {
            RegisterIndex reg = register_from_string(reg1_str);
            uint32_t value = parse_value_double(reg2_str);
            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = value;
            program_counter += 4;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            char val_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(val_hex + i * 3, "%02X ", memory[instruction_start_address + 2 + i]); }
            strcat(binary_output, val_hex);
            break;
        }
        case OP_MATH_CLAMP: {
            RegisterIndex reg_dest = register_from_string(reg1_str);
            RegisterIndex reg_val = register_from_string(reg1_str);
            RegisterIndex reg_min = register_from_string(reg2_str);
            RegisterIndex reg_max = register_from_string(reg3_str);

            memory[program_counter++] = (uint8_t)reg_dest;
            memory[program_counter++] = (uint8_t)reg_min;
            memory[program_counter++] = (uint8_t)reg_max;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_dest_hex[8]; sprintf(reg_dest_hex, "%02X ", reg_dest); strcat(binary_output, reg_dest_hex);
            char reg_min_hex[8]; sprintf(reg_min_hex, "%02X ", reg_min); strcat(binary_output, reg_min_hex);
            char reg_max_hex[8]; sprintf(reg_max_hex, "%02X ", reg_max); strcat(binary_output, reg_max_hex);
            break;
        }
        case OP_MATH_LERP: {
            RegisterIndex reg_dest = register_from_string(reg1_str);
            RegisterIndex reg_start = register_from_string(reg2_str);
            RegisterIndex reg_end = register_from_string(reg3_str);
            RegisterIndex reg_step = register_from_string(reg4_str);

            memory[program_counter++] = (uint8_t)reg_dest;
            memory[program_counter++] = (uint8_t)reg_start;
            memory[program_counter++] = (uint8_t)reg_end;
            memory[program_counter++] = (uint8_t)reg_step;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_dest_hex[8]; sprintf(reg_dest_hex, "%02X ", reg_dest); strcat(binary_output, reg_dest_hex);
            char reg_start_hex[8]; sprintf(reg_start_hex, "%02X ", reg_start); strcat(binary_output, reg_start_hex);
            char reg_end_hex[8]; sprintf(reg_end_hex, "%02X ", reg_end); strcat(binary_output, reg_end_hex);
            char reg_step_hex[8]; sprintf(reg_step_hex, "%02X ", reg_step); strcat(binary_output, reg_step_hex);
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
        case OP_CALL_ADDR:
        case OP_DISK_CREATE_IMAGE: {
            uint32_t address = parse_address(reg1_str);
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(addr_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); }
            strcat(binary_output, addr_hex);
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

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr1_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(addr1_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); }
            strcat(binary_output, addr1_hex);
            char addr2_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(addr2_hex + i * 3, "%02X ", memory[instruction_start_address + 5 + i]); }
            strcat(binary_output, addr2_hex);
            break;
        }
        case OP_MEM_SET_MEM_REG_REG: {
            uint32_t dest_addr = decode_address();
            RegisterIndex reg1 = decode_register();
            RegisterIndex reg2 = decode_register();
            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg1;
            memory[program_counter++] = (uint8_t)reg2;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr_hex[16] = "";
            for (int i = 0; i < 4; ++i) { sprintf(addr_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); }
            strcat(binary_output, addr_hex);
            char reg1_hex[8]; sprintf(reg1_hex, "%02X ", reg1); strcat(binary_output, reg1_hex);
            char reg2_hex[8]; sprintf(reg2_hex, "%02X ", reg2); strcat(binary_output, reg2_hex);

            break;
        }
        case OP_SYS_READ_STRING: {
            RegisterIndex reg1 = register_from_string(reg1_str);
            RegisterIndex reg2 = register_from_string(reg2_str);
            memory[program_counter++] = (uint8_t)reg1;
            memory[program_counter++] = (uint8_t)reg2;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg1_hex[8]; sprintf(reg1_hex, "%02X ", reg1); strcat(binary_output, reg1_hex);
            char reg2_hex[8]; sprintf(reg2_hex, "%02X ", reg2); strcat(binary_output, reg2_hex);
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

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            char addr1_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr1_hex + i * 3, "%02X ", memory[instruction_start_address + 2 + i]); } strcat(binary_output, addr1_hex);
            char addr2_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr2_hex + i * 3, "%02X ", memory[instruction_start_address + 6 + i]); } strcat(binary_output, addr2_hex);

            break;
        }
        case OP_STR_NCPY_MEM_MEM_REG: {
            uint32_t dest_addr = parse_address(reg1_str);
            uint32_t src_addr = parse_address(reg2_str);
            RegisterIndex reg = register_from_string(reg3_str);

            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = src_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr1_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr1_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); } strcat(binary_output, addr1_hex);
            char addr2_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr2_hex + i * 3, "%02X ", memory[instruction_start_address + 5 + i]); } strcat(binary_output, addr2_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);

            break;
        }
        case OP_STR_NCAT_MEM_MEM_REG: {
            uint32_t dest_addr = parse_address(reg1_str);
            uint32_t src_addr = parse_address(reg2_str);
            RegisterIndex reg = register_from_string(reg3_str);

            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = src_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr1_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr1_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); } strcat(binary_output, addr1_hex);
            char addr2_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr2_hex + i * 3, "%02X ", memory[instruction_start_address + 5 + i]); } strcat(binary_output, addr2_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);

            break;
        }
        case OP_MEM_CPY_MEM_MEM_REG: {
            uint32_t dest_addr = parse_address(reg1_str);
            uint32_t src_addr = parse_address(reg2_str);
            RegisterIndex reg = register_from_string(reg3_str);

            *(uint32_t*)&memory[program_counter] = dest_addr;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = src_addr;
            program_counter += 4;
            memory[program_counter++] = (uint8_t)reg;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr1_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr1_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); } strcat(binary_output, addr1_hex);
            char addr2_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr2_hex + i * 3, "%02X ", memory[instruction_start_address + 5 + i]); } strcat(binary_output, addr2_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);

            break;
        }
        case OP_STR_CHR_REG_MEM_VAL: {
            RegisterIndex reg = register_from_string(reg1_str);
            uint32_t address = parse_address(reg2_str);
            uint32_t value = parse_address(reg3_str);

            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = address;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = value;
            program_counter += 4;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            char addr_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr_hex + i * 3, "%02X ", memory[instruction_start_address + 2 + i]); } strcat(binary_output, addr_hex);
            char val_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(val_hex + i * 3, "%02X ", memory[instruction_start_address + 6 + i]); } strcat(binary_output, val_hex);

            break;
        }
        case OP_STR_STR_REG_MEM_MEM: {
            RegisterIndex reg = register_from_string(reg1_str);
            uint32_t addr1 = parse_address(reg2_str);
            uint32_t addr2 = parse_address(reg3_str);

            memory[program_counter++] = (uint8_t)reg;
            *(uint32_t*)&memory[program_counter] = addr1;
            program_counter += 4;
            *(uint32_t*)&memory[program_counter] = addr2;
            program_counter += 4;

            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char reg_hex[8]; sprintf(reg_hex, "%02X ", reg); strcat(binary_output, reg_hex);
            char addr1_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr1_hex + i * 3, "%02X ", memory[instruction_start_address + 2 + i]); } strcat(binary_output, addr1_hex);
            char addr2_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr2_hex + i * 3, "%02X ", memory[instruction_start_address + 6 + i]); } strcat(binary_output, addr2_hex);

            break;
        }
        case OP_STR_ITOA_MEM_REG_REG:
        case OP_STR_FMT_MEM_MEM_REG_REG:
        case OP_STR_SUBSTR_MEM_MEM_REG_REG:
        case OP_MEM_SET_MEM_REG_VAL:
        case OP_SYS_SET_CURSOR_POS:
        case OP_SYS_GET_CURSOR_POS:
        case OP_SYS_NUMBER_TO_STRING:
        case OP_DISK_READ_SECTOR_MEM_REG_REG:
        case OP_DISK_WRITE_SECTOR_MEM_REG_REG:
        {
            RegisterIndex reg1, reg2, reg3 = REG_INVALID, reg4 = REG_INVALID;
            uint32_t address = decode_address();
            reg1 = decode_register();
            reg2 = decode_register();
            if (opcode == OP_STR_SUBSTR_MEM_MEM_REG_REG || opcode == OP_STR_FMT_MEM_MEM_REG_REG || opcode == OP_MATH_LERP || opcode == OP_DISK_READ_SECTOR_MEM_REG_REG || opcode == OP_DISK_WRITE_SECTOR_MEM_REG_REG) reg3 = decode_register();
            if (opcode == OP_STR_FMT_MEM_MEM_REG_REG || opcode == OP_MATH_LERP) reg4 = decode_register();


            program_counter = instruction_start_address + 1;
            *(uint32_t*)&memory[program_counter] = address; program_counter += 4;
            memory[program_counter++] = (uint8_t)reg1;
            memory[program_counter++] = (uint8_t)reg2;
            if (reg3 != REG_INVALID) memory[program_counter++] = (uint8_t)reg3;
            if (reg4 != REG_INVALID) memory[program_counter++] = (uint8_t)reg4;


            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            char addr_hex[16] = ""; for (int i = 0; i < 4; ++i) { sprintf(addr_hex + i * 3, "%02X ", memory[instruction_start_address + 1 + i]); } strcat(binary_output, addr_hex);
            char reg1_hex[8]; sprintf(reg1_hex, "%02X ", reg1); strcat(binary_output, reg1_hex);
            char reg2_hex[8]; sprintf(reg2_hex, "%02X ", reg2); strcat(binary_output, reg2_hex);
            if (reg3 != REG_INVALID) { char reg3_hex[8]; sprintf(reg3_hex, "%02X ", reg3); strcat(binary_output, reg3_hex); }
            if (reg4 != REG_INVALID) { char reg4_hex[8]; sprintf(reg4_hex, "%02X ", reg4); strcat(binary_output, reg4_hex); }

            break;
        }
        case OP_SYS_RESET_TEXT_COLOR:
        case OP_SYS_PRINT_NEWLINE:
        case OP_SYS_CLEAR_SCREEN:
        case OP_RET:
        case OP_NOP:
        case OP_HLT:
        case OP_DISK_FORMAT_DISK:
        {
            char opcode_hex[8]; sprintf(opcode_hex, "%02X ", opcode); strcat(binary_output, opcode_hex);
            break;
        }
        default:
            fprintf(stderr, "Assembler Error: Unhandled opcode in assembler switch on line %d (Pass 2).\n", line_number);
            fclose(asm_file);
            fclose(rom_file);
            fclose(lst_file);
            return -1;
        }
        fprintf(lst_file, "%-9d| %-8X | %-30s | %-20s | %s", line_number, instruction_start_address + rom_offset, mnemonic_output, binary_output, "\n");
        line_number++;
        current_address = program_counter + rom_offset;
    }

    fwrite(memory + rom_offset, 1, data_pointer - rom_offset, rom_file);

    fclose(asm_file);
    fclose(rom_file);
    fclose(lst_file);
    printf("Successfully assembled '%s' to '%s' and '%s'\n", asm_filename, rom_filename, lst_filename);
    return 0;
}

int load_rom(const char* rom_filename) {
    FILE* rom_file = fopen(rom_filename, "rb");
    if (!rom_file) {
        perror("Error opening ROM file for reading");
        return -1;
    }

    memset(memory, 0, MEMORY_SIZE);
    fseek(rom_file, 0, SEEK_END);
    long rom_size = ftell(rom_file);
    rewind(rom_file);

    if (rom_size > MEMORY_SIZE) {
        fprintf(stderr, "Error: ROM file is too large to load into memory.\n");
        fclose(rom_file);
        return -1;
    }

    size_t bytes_read = fread(memory, 1, rom_size, rom_file);
    fclose(rom_file);
    printf("Loaded %zu bytes from '%s'\n", bytes_read, rom_filename);
    return 0;
}

int main(int argc, char* argv[]) {
    char choice;
    char filename[256];

#ifdef _WIN32
    CreateDirectory(L"img", NULL);
#else
    mkdir("img", 0777);
#endif


    while (1) {
        printf("\nChoose action:\n");
        printf("1. Assemble .asm to .rom and .lst\n");
        printf("2. Run .rom\n");
        printf("3. Exit\n");
        printf("Enter choice (1-3): ");
        scanf(" %c", &choice);

        switch (choice) {
        case '1':
            printf("Enter assembly filename (.asm): ");
            scanf("%255s", filename);
            if (assemble_program(filename, "output.rom") == 0) {
                printf("Assembly successful. ROM file 'output.rom' and listing file 'output.rom.lst' created.\n");
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
