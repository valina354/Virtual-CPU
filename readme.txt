**CPU Version:** 5
**Memory:** 16MB (16384 * 1024 bytes)
**Registers:** 32 General Purpose Registers (R0-R31), Stack Pointer (SP), Zero Flag (ZF), Sign Flag (SF), Carry Flag (CF), Overflow Flag (OF).  Registers are 64-bit floating-point numbers internally, but many instructions operate on 32-bit integers after casting.
**Data Types:** 64-bit floating-point (double), 32-bit unsigned integer (uint32_t), 32-bit signed integer (int32_t), 8-bit character (char). Memory is byte-addressable.
**Addressing Modes:**
    * **Register Direct:** Operands are registers (e.g., R0, SP).
    * **Immediate Value:** Operands are constant values embedded in the instruction stream (double or uint32_t).
    * **Memory Direct:** Operands are memory addresses specified directly in the instruction (e.g., `[1000]`).
    * **Label Address:** Operands can be labels which are resolved to memory addresses during assembly.

**Flags:**
    * **ZF (Zero Flag):** Set if the result of an operation is zero.
    * **SF (Sign Flag):** Set if the result of an operation is negative (most significant bit is set for integer operations, negative for floating-point).
    * **CF (Carry Flag):** Set if an arithmetic operation results in a carry or borrow (unsigned overflow/underflow), or for shift/rotate operations.
    * **OF (Overflow Flag):** Set if a signed arithmetic operation results in overflow (signed overflow/underflow).

**Instruction Format:**

Instructions are byte-encoded.  The first byte is the opcode.  Operands follow the opcode, their format depending on the specific instruction (registers are encoded as single bytes, values as 4 or 8 bytes, addresses as 4 bytes).

**Instruction Set:**

| Mnemonic      | Opcode (Hex) | Operands                                 | Description                                                                      | Flags Affected |
|---------------|--------------|------------------------------------------|----------------------------------------------------------------------------------|----------------|
| **Data Transfer Instructions** |              |                                          |                                                                                |                |
| NOP           | 0x00         | None                                     | No operation.                                                                    | None           |
| MOV Reg, Val  | 0x01         | `Reg_dest`, `Value(double)`            | Move immediate 64-bit floating-point value into register.                        | None           |
| MOV Reg, Reg  | 0x02         | `Reg_dest`, `Reg_src`                  | Move register value to another register.                                        | None           |
| MOV Reg, Mem  | 0x03         | `Reg_dest`, `Address(uint32_t)`         | Move 64-bit floating-point value from memory to register.                       | None           |
| MOV Mem, Reg  | 0x04         | `Address(uint32_t)`, `Reg_src`         | Move register value to memory (64-bit floating-point).                         | None           |
| MOVZX Reg, Reg| 0x1C         | `Reg_dest`, `Reg_src`                  | Move and Zero-Extend: Copy lower 32 bits from source register to destination register and zero-extend to 64 bits. | None           |
| MOVZX Reg, Mem| 0x1D         | `Reg_dest`, `Address(uint32_t)`         | Move and Zero-Extend: Copy 32 bits from memory to register and zero-extend to 64 bits. | None           |
| MOVSX Reg, Reg| 0x1E         | `Reg_dest`, `Reg_src`                  | Move and Sign-Extend: Copy lower 32 bits from source register to destination register and sign-extend to 64 bits. | None           |
| MOVSX Reg, Mem| 0x1F         | `Reg_dest`, `Address(uint32_t)`         | Move and Sign-Extend: Copy 32 bits from memory to register and sign-extend to 64 bits. | None           |
| LEA Reg, Mem  | 0x20         | `Reg_dest`, `Address(uint32_t)`         | Load Effective Address: Load the memory address (not the value at that address) into the register. | None           |
| XCHG Reg, Reg | 0x3E         | `Reg_dest`, `Reg_src`                  | Exchange the values of two registers.                                            | None           |

| **Arithmetic Instructions** |              |                                          |                                                                                |                |
| ADD Reg, Reg  | 0x05         | `Reg_dest`, `Reg_src`                  | Add register value to destination register (floating-point).                    | ZF, SF, CF, OF |
| ADD Reg, Val  | 0x06         | `Reg_dest`, `Value(double)`            | Add immediate 64-bit floating-point value to register.                         | ZF, SF, CF, OF |
| SUB Reg, Reg  | 0x07         | `Reg_dest`, `Reg_src`                  | Subtract register value from destination register (floating-point).               | ZF, SF, CF, OF |
| SUB Reg, Val  | 0x08         | `Reg_dest`, `Value(double)`            | Subtract immediate 64-bit floating-point value from register.                    | ZF, SF, CF, OF |
| MUL Reg, Reg  | 0x09         | `Reg_dest`, `Reg_src`                  | Multiply register values (floating-point).                                     | ZF, SF, CF, OF |
| MUL Reg, Val  | 0x0A         | `Reg_dest`, `Value(double)`            | Multiply register by immediate 64-bit floating-point value.                     | ZF, SF, CF, OF |
| DIV Reg, Reg  | 0x0B         | `Reg_dest`, `Reg_src`                  | Divide destination register by register value (floating-point).                | ZF, SF, CF, OF |
| DIV Reg, Val  | 0x0C         | `Reg_dest`, `Value(double)`            | Divide register by immediate 64-bit floating-point value.                     | ZF, SF, CF, OF |
| MOD Reg, Reg  | 0x0D         | `Reg_dest`, `Reg_src`                  | Modulo (remainder) of destination register divided by register value (floating-point). | ZF, SF, CF, OF |
| MOD Reg, Val  | 0x0E         | `Reg_dest`, `Value(double)`            | Modulo (remainder) of register divided by immediate 64-bit floating-point value. | ZF, SF, CF, OF |
| IMUL Reg, Reg | 0x1A         | `Reg_dest`, `Reg_src`                  | Signed integer multiplication (32-bit integers). Result stored as double.       | ZF, SF, CF, OF |
| IDIV Reg, Reg | 0x1B         | `Reg_dest`, `Reg_src`                  | Signed integer division (32-bit integers). Result stored as double.             | ZF, SF, CF, OF |
| INC Reg       | 0x2B         | `Reg_dest`                             | Increment register value by 1 (floating-point).                                | ZF, SF, OF     |
| DEC Reg       | 0x2C         | `Reg_dest`                             | Decrement register value by 1 (floating-point).                                | ZF, SF, OF     |
| INC Mem       | 0x2D         | `Address(uint32_t)`                     | Increment 64-bit floating-point value in memory by 1.                         | None           |
| DEC Mem       | 0x2E         | `Address(uint32_t)`                     | Decrement 64-bit floating-point value in memory by 1.                         | None           |
| NEG Reg       | 0x19         | `Reg_dest`                             | Negate (two's complement) the value in register (integer). Result stored as double. | ZF, SF, CF, OF |

| **Logical Instructions** |              |                                          |                                                                                |                |
| AND Reg, Reg  | 0x0F         | `Reg_dest`, `Reg_src`                  | Bitwise AND of register values (integer). Result stored as double.             | ZF, SF, CF, OF |
| AND Reg, Val  | 0x10         | `Reg_dest`, `Value(uint32_t)`          | Bitwise AND of register and immediate 32-bit unsigned integer. Result stored as double. | ZF, SF, CF, OF |
| OR Reg, Reg   | 0x11         | `Reg_dest`, `Reg_src`                  | Bitwise OR of register values (integer). Result stored as double.              | ZF, SF, CF, OF |
| OR Reg, Val   | 0x12         | `Reg_dest`, `Value(uint32_t)`          | Bitwise OR of register and immediate 32-bit unsigned integer. Result stored as double. | ZF, SF, CF, OF |
| XOR Reg, Reg  | 0x13         | `Reg_dest`, `Reg_src`                  | Bitwise XOR of register values (integer). Result stored as double.             | ZF, SF, CF, OF |
| XOR Reg, Val  | 0x14         | `Reg_dest`, `Value(uint32_t)`          | Bitwise XOR of register and immediate 32-bit unsigned integer. Result stored as double. | ZF, SF, CF, OF |
| NOT Reg       | 0x15         | `Reg_dest`                             | Bitwise NOT (one's complement) of register value (integer). Result stored as double. | ZF, SF, CF, OF |
| TEST Reg, Reg | 0x18         | `Reg_op1`, `Reg_op2`                   | Bitwise AND of operands, but result is discarded. Only flags are updated (integer). | ZF, SF, CF, OF |
| TEST Reg, Val | 0x19         | `Reg_op1`, `Value(uint32_t)`          | Bitwise AND of register and immediate value, flags updated. Result discarded (integer). | ZF, SF, CF, OF |

| **Shift and Rotate Instructions** |              |                                          |                                                                                |                |
| SHL Reg, Reg  | 0x2F         | `Reg_dest`, `Reg_count`                | Shift Left Logical: Shift bits in register left by count from register.          | ZF, SF, CF, OF |
| SHL Reg, Val  | 0x30         | `Reg_dest`, `Value(uint32_t)`          | Shift Left Logical: Shift bits in register left by immediate value.            | ZF, SF, CF, OF |
| SHR Reg, Reg  | 0x31         | `Reg_dest`, `Reg_count`                | Shift Right Logical: Shift bits in register right by count from register.         | ZF, SF, CF, OF |
| SHR Reg, Val  | 0x32         | `Reg_dest`, `Value(uint32_t)`          | Shift Right Logical: Shift bits in register right by immediate value.           | ZF, SF, CF, OF |
| SAR Reg, Reg  | 0x33         | `Reg_dest`, `Reg_count`                | Shift Right Arithmetic: Shift bits in register right by count, sign-extending.  | ZF, SF, CF, OF |
| SAR Reg, Val  | 0x34         | `Reg_dest`, `Value(uint32_t)`          | Shift Right Arithmetic: Shift bits in register right by immediate value, sign-extending. | ZF, SF, CF, OF |
| ROL Reg, Reg  | 0x35         | `Reg_dest`, `Reg_count`                | Rotate Left: Rotate bits in register left by count from register.              | ZF, SF, CF, OF |
| ROL Reg, Val  | 0x36         | `Reg_dest`, `Value(uint32_t)`          | Rotate Left: Rotate bits in register left by immediate value.                | ZF, SF, CF, OF |
| ROR Reg, Reg  | 0x37         | `Reg_dest`, `Reg_count`                | Rotate Right: Rotate bits in register right by count from register.             | ZF, SF, CF, OF |
| ROR Reg, Val  | 0x38         | `Reg_dest`, `Value(uint32_t)`          | Rotate Right: Rotate bits in register right by immediate value.               | ZF, SF, CF, OF |
| BSWAP Reg     | 0x3F         | `Reg_dest`                             | Byte Swap: Reverses the byte order of a 32-bit value in the register.           | None           |

| **Comparison Instructions** |              |                                          |                                                                                |                |
| CMP Reg, Reg  | 0x16         | `Reg_op1`, `Reg_op2`                   | Compare register values (floating-point). Sets flags based on `Reg_op1 - Reg_op2`. | ZF, SF, CF, OF |
| CMP Reg, Val  | 0x17         | `Reg_op1`, `Value(double)`            | Compare register to immediate 64-bit floating-point value. Sets flags based on `Reg_op1 - Value`. | ZF, SF, CF, OF |
| SETZ Reg      | 0x40         | `Reg_dest`                             | Set if Zero: Set register to 1 if ZF is set, otherwise 0.                       | None           |
| SETNZ Reg     | 0x41         | `Reg_dest`                             | Set if Not Zero: Set register to 1 if ZF is not set, otherwise 0.                   | None           |

| **Control Flow Instructions** |              |                                          |                                                                                |                |
| JMP Address   | 0x21         | `Address(uint32_t)`                     | Unconditional Jump: Jump to the specified address.                             | None           |
| JMP NZ Address| 0x22         | `Address(uint32_t)`                     | Jump if Not Zero: Jump to address if ZF is not set.                             | None           |
| JMP Z Address | 0x23         | `Address(uint32_t)`                     | Jump if Zero: Jump to address if ZF is set.                                     | None           |
| JMP S Address | 0x24         | `Address(uint32_t)`                     | Jump if Sign: Jump to address if SF is set (negative result).                  | None           |
| JMP NS Address| 0x25         | `Address(uint32_t)`                     | Jump if Not Sign: Jump to address if SF is not set (non-negative result).      | None           |
| JMP C Address | 0x26         | `Address(uint32_t)`                     | Jump if Carry: Jump to address if CF is set.                                    | None           |
| JMP NC Address| 0x27         | `Address(uint32_t)`                     | Jump if Not Carry: Jump to address if CF is not set.                               | None           |
| JMP O Address | 0x28         | `Address(uint32_t)`                     | Jump if Overflow: Jump to address if OF is set.                                   | None           |
| JMP NO Address| 0x29         | `Address(uint32_t)`                     | Jump if Not Overflow: Jump to address if OF is not set.                              | None           |
| JMP GE Address| 0x2A         | `Address(uint32_t)`                     | Jump if Greater or Equal (Signed): Jump if SF == OF.                             | None           |
| JMP LE Address| 0x2B         | `Address(uint32_t)`                     | Jump if Less or Equal (Signed): Jump if ZF == 1 or SF != OF.                  | None           |
| JMP G Address | 0x2C         | `Address(uint32_t)`                     | Jump if Greater (Signed): Jump if ZF == 0 and SF == OF.                        | None           |
| JMP L Address | 0x2D         | `Address(uint32_t)`                     | Jump if Less (Signed): Jump if ZF == 0 and SF != OF.                         | None           |
| HLT           | 0x2E         | None                                     | Halt execution.                                                                  | None           |
| CALL Address  | 0x3B         | `Address(uint32_t)`                     | Call Subroutine: Push return address onto stack and jump to address.             | None           |
| RET           | 0x3C         | None                                     | Return from Subroutine: Pop return address from stack and jump to it.           | None           |

| **Stack Operations** |              |                                          |                                                                                |                |
| PUSH Reg      | 0x39         | `Reg_src`                              | Push register value onto the stack (64-bit floating-point).                      | None           |
| POP Reg       | 0x3A         | `Reg_dest`                             | Pop value from the stack into register (64-bit floating-point).                  | None           |
| PUSHA         | 0x42         | None                                     | Push All General Purpose Registers (R0-R31) onto the stack.                     | None           |
| POPA          | 0x43         | None                                     | Pop All General Purpose Registers (R31-R0) from the stack.                      | None           |
| PUSHFD        | 0x44         | None                                     | Push Flags: Push the flag register values (ZF, SF, CF, OF) onto the stack as a 32-bit integer. | None           |
| POPFD         | 0x45         | None                                     | Pop Flags: Pop a 32-bit integer from the stack and set the flag registers (ZF, SF, CF, OF) accordingly. | None           |

| **Math Standard Library** |              |                                          |                                                                                |                |
| math.add Reg, Reg | 0x46         | `Reg_dest`, `Reg_src`                  | Floating-point addition.                                                         | ZF, SF, CF, OF |
| math.sub Reg, Reg | 0x47         | `Reg_dest`, `Reg_src`                  | Floating-point subtraction.                                                      | ZF, SF, CF, OF |
| math.mul Reg, Reg | 0x48         | `Reg_dest`, `Reg_src`                  | Floating-point multiplication.                                                   | ZF, SF, CF, OF |
| math.div Reg, Reg | 0x49         | `Reg_dest`, `Reg_src`                  | Floating-point division.                                                         | ZF, SF, CF, OF |
| math.mod Reg, Reg | 0x4A         | `Reg_dest`, `Reg_src`                  | Floating-point modulo.                                                           | ZF, SF, CF, OF |
| math.abs Reg    | 0x4B         | `Reg_dest`                             | Floating-point absolute value.                                                   | ZF, SF, CF, OF |
| math.sin Reg    | 0x4C         | `Reg_dest`                             | Floating-point sine.                                                             | ZF, SF, CF, OF |
| math.cos Reg    | 0x4D         | `Reg_dest`                             | Floating-point cosine.                                                           | ZF, SF, CF, OF |
| math.tan Reg    | 0x4E         | `Reg_dest`                             | Floating-point tangent.                                                          | ZF, SF, CF, OF |
| math.asin Reg   | 0x4F         | `Reg_dest`                             | Floating-point arcsine.                                                          | ZF, SF, CF, OF |
| math.acos Reg   | 0x50         | `Reg_dest`                             | Floating-point arccosine.                                                         | ZF, SF, CF, OF |
| math.atan Reg   | 0x51         | `Reg_dest`                             | Floating-point arctangent.                                                        | ZF, SF, CF, OF |
| math.pow Reg, Reg| 0x52         | `Reg_dest`, `Reg_src`                  | Floating-point power (base raised to exponent).                                  | ZF, SF, CF, OF |
| math.sqrt Reg   | 0x53         | `Reg_dest`                             | Floating-point square root.                                                      | ZF, SF, CF, OF |
| math.log Reg    | 0x54         | `Reg_dest`                             | Floating-point natural logarithm (base e).                                        | ZF, SF, CF, OF |
| math.exp Reg    | 0x55         | `Reg_dest`                             | Floating-point exponential (e raised to the power).                               | ZF, SF, CF, OF |
| math.floor Reg  | 0x56         | `Reg_dest`                             | Floating-point floor (round down to nearest integer).                            | ZF, SF, CF, OF |
| math.ceil Reg   | 0x57         | `Reg_dest`                             | Floating-point ceiling (round up to nearest integer).                             | ZF, SF, CF, OF |
| math.round Reg  | 0x58         | `Reg_dest`                             | Floating-point round to nearest integer.                                         | ZF, SF, CF, OF |
| math.min Reg, Reg | 0x59        | `Reg_dest`, `Reg_src`                  | Floating-point minimum of two values.                                            | ZF, SF, CF, OF |
| math.max Reg, Reg | 0x5A        | `Reg_dest`, `Reg_src`                  | Floating-point maximum of two values.                                            | ZF, SF, CF, OF |
| math.neg Reg    | 0x5B         | `Reg_dest`                             | Floating-point negation.                                                         | ZF, SF, CF, OF |
| math.atan2 Reg, Reg| 0x5C     | `Reg_dest`, `Reg_src`                  | Floating-point arctangent of y/x, using signs to determine quadrant.             | ZF, SF, CF, OF |
| math.log10 Reg  | 0x5D         | `Reg_dest`                             | Floating-point base 10 logarithm.                                                | ZF, SF, CF, OF |
| math.clamp Reg, Reg, Reg| 0x5E | `Reg_val`, `Reg_min`, `Reg_max`       | Clamp a value within a specified range. `Reg_val = max(Reg_min, min(Reg_val, Reg_max))`. | None           |
| math.lerp Reg, Reg, Reg, Reg| 0x5F| `Reg_dest`, `Reg_start`, `Reg_end`, `Reg_step`| Linear Interpolation: `Reg_dest = Reg_start + (Reg_end - Reg_start) * Reg_step`. | None           |

| **String Standard Library** |              |                                          |                                                                                |                |
| str.len Reg, Mem| 0x60         | `Reg_dest`, `Address(uint32_t)`         | String Length: Get the length of a null-terminated string in memory. Result in register. | ZF, SF         |
| str.cpy Mem, Mem| 0x61         | `Address_dest(uint32_t)`, `Address_src(uint32_t)`| String Copy: Copy a null-terminated string from source to destination memory. | None           |
| str.cat Mem, Mem| 0x62         | `Address_dest(uint32_t)`, `Address_src(uint32_t)`| String Concatenate: Append a null-terminated string from source to destination memory. | None           |
| str.cmp Reg, Mem, Mem| 0x63    | `Reg_dest`, `Address1(uint32_t)`, `Address2(uint32_t)`| String Compare: Compare two null-terminated strings in memory. Result in register (strcmp-like). | ZF, SF         |
| str.ncpy Mem, Mem, Reg| 0x64    | `Address_dest(uint32_t)`, `Address_src(uint32_t)`, `Reg_count`| String N Copy: Copy at most `Reg_count` bytes of a string.                     | None           |
| str.ncat Mem, Mem, Reg| 0x65    | `Address_dest(uint32_t)`, `Address_src(uint32_t)`, `Reg_count`| String N Concatenate: Append at most `Reg_count` bytes of a string.          | None           |
| str.toupper Mem | 0x66         | `Address(uint32_t)`                     | String to Upper Case: Convert a null-terminated string in memory to uppercase. | None           |
| str.tolower Mem | 0x67         | `Address(uint32_t)`                     | String to Lower Case: Convert a null-terminated string in memory to lowercase. | None           |
| str.chr Reg, Mem, Val| 0x68    | `Reg_dest`, `Address(uint32_t)`, `Value(char)`| String Character Search: Find the first occurrence of a character in a string. Result index in register (-1 if not found). | ZF, SF         |
| str.str Reg, Mem, Mem| 0x69    | `Reg_dest`, `Address1(uint32_t)`, `Address2(uint32_t)`| String String Search: Find the first occurrence of a substring within a string. Result index in register (-1 if not found). | ZF, SF         |
| str.atoi Reg, Mem| 0x6A         | `Reg_dest`, `Address(uint32_t)`         | String to Integer (atoi): Convert a string to an integer. Result in register. | ZF, SF         |
| str.itoa Mem, Reg, Reg| 0x6B    | `Address_dest(uint32_t)`, `Reg_val`, `Reg_buffer_size`| Integer to String (itoa): Convert an integer to a string and store in memory buffer. | None           |
| str.substr Mem, Mem, Reg, Reg| 0x6C| `Address_dest(uint32_t)`, `Address_src(uint32_t)`, `Reg_start_index`, `Reg_length`| Substring: Extract a substring from a string in memory.                      | None           |
| str.fmt Mem, Mem, Reg, Reg| 0x6D| `Address_dest(uint32_t)`, `Address_fmt(uint32_t)`, `Reg_arg1`, `Reg_arg2`| String Format (sprintf-like): Format a string using format specifier string and arguments. | None           |

| **Memory Standard Library** |              |                                          |                                                                                |                |
| mem.cpy Mem, Mem, Reg| 0x6E    | `Address_dest(uint32_t)`, `Address_src(uint32_t)`, `Reg_count`| Memory Copy (memcpy): Copy `Reg_count` bytes from source to destination memory. | None           |
| mem.set Mem, Reg, Val| 0x6F    | `Address_dest(uint32_t)`, `Reg_value`, `Value_count(uint32_t)`| Memory Set (memset): Set `Value_count` bytes in memory to `Reg_value`.        | None           |
| mem.set Mem, Reg, Reg| 0x70    | `Address_dest(uint32_t)`, `Reg_value`, `Reg_count`| Memory Set (memset): Set `Reg_count` bytes in memory to `Reg_value`.        | None           |
| mem.clear Mem   | 0x71         | `Address(uint32_t)`                     | Memory Clear (memset to 0): Clear a memory buffer (size determined by buffer definition). | None           |

| **System Standard Library** |              |                                          |                                                                                |                |
| sys.print_char Reg| 0x72         | `Reg_char`                             | Print Character: Print the character in the register to the console.             | None           |
| sys.clear_screen| 0x73         | None                                     | Clear Screen: Clear the console screen.                                          | None           |
| sys.print_string Reg| 0x74      | `Reg_address`                          | Print String: Print a null-terminated string from memory address to the console.  | None           |
| sys.newline     | 0x75         | None                                     | Print Newline: Print a newline character to the console.                        | None           |
| sys.set_cursor_pos Reg, Reg| 0x76| `Reg_x`, `Reg_y`                       | Set Cursor Position: Set the console cursor position to (X, Y) coordinates.    | None           |
| sys.get_cursor_pos Reg, Reg| 0x77| `Reg_x_dest`, `Reg_y_dest`             | Get Cursor Position: Get the current console cursor position and store in registers. | None           |
| sys.set_text_color Reg| 0x78     | `Reg_color_code`                       | Set Text Color: Set the text color using a color code.                          | None           |
| sys.reset_text_color| 0x79     | None                                     | Reset Text Color: Reset the text color to default.                             | None           |
| sys.print_number_dec Reg| 0x7A  | `Reg_number`                           | Print Number Decimal: Print a floating-point number in decimal format.           | None           |
| sys.print_number_hex Reg| 0x7B  | `Reg_number`                           | Print Number Hexadecimal: Print a 32-bit unsigned integer in hexadecimal format. | None           |
| sys.number_to_string Reg, Reg, Reg| 0x7C| `Reg_number`, `Reg_address_dest`, `Reg_buffer_size`| Number to String: Convert a 32-bit unsigned integer to string and store in memory. | None           |
| sys.read_char   Reg| 0x7D         | `Reg_char_dest`                        | Read Character: Read a single character from console input and store in register. | None           |
| sys.read_string Reg, Reg| 0x7E     | `Reg_address_dest`, `Reg_max_length`   | Read String: Read a string from console input into memory buffer.              | None           |
| sys.get_key_press Reg| 0x7F     | `Reg_key_dest`                         | Get Key Press: Check for a key press without blocking. Returns key code if pressed, 0 otherwise. | None           |
| sys.cpu_ver Reg | 0x80         | `Reg_dest`                             | Get CPU Version: Get the CPU version number.                                   | None           |
| sys.wait Reg    | 0x81         | `Reg_milliseconds`                     | Wait: Pause execution for specified milliseconds.                               | None           |
| sys.time Reg    | 0x82         | `Reg_time_dest`                        | Get Time: Get the current time in seconds since epoch (UNIX timestamp) as a double. | None           |

| **Disk Standard Library** |              |                                          |                                                                                |                |
| disk.get_size Reg| 0x83         | `Reg_size_dest`                        | Get Disk Size: Get the size of the virtual disk image in bytes. Result in register. | None           |
| disk.read_sector Reg, Reg, Mem| 0x84| `Reg_sector_number`, `Reg_count`, `Address_mem(uint32_t)`| Read Sector: Read sectors from disk to memory. Sector number and count from registers. | None           |
| disk.write_sector Reg, Reg, Mem| 0x85| `Reg_sector_number`, `Reg_count`, `Address_mem(uint32_t)`| Write Sector: Write sectors from memory to disk. Sector number and count from registers. | None           |
| disk.create_image| 0x86         | None                                     | Create Disk Image: Create a new virtual disk image file.                         | None           |
| disk.get_volume_label Mem| 0x87 | `Address_mem(uint32_t)`                 | Get Volume Label: Get the volume label of the disk and store it in memory.       | None           |
| disk.set_volume_label Mem| 0x88 | `Address_mem(uint32_t)`                 | Set Volume Label: Set the volume label of the disk from memory.                 | None           |
| disk.format_disk	0x89| 	None| 	Format Disk: Formats the virtual disk image, overwriting all data. Creates a new header and fills data area with zeros.|	None    |


**Register Encoding:**

* Registers are encoded as single bytes following the opcode where required.
* Register indices:
    * R0-R31: 0x00 - 0x1F
    * SP:     0x20
    * ZF:     0x21
    * SF:     0x22
    * CF:     0x23
    * OF:     0x24

**Value Encoding:**

* **Double (64-bit floating-point):** 8 bytes, little-endian.
* **UInt32 (32-bit unsigned integer):** 4 bytes, little-endian.

**Address Encoding:**

* **Memory Address (32-bit):** 4 bytes, little-endian.
