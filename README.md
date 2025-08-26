# ChipEmu80

ChipEmu80 is a minimal Z80 CPU emulator implemented in C. It allows you to run Z80 binary programs in a simplified environment with video output.

---

## Features
- Emulates basic Z80 registers (A, B, C, D, E, F, H, L, SP, PC, CR)
- Supports a subset of Z80 opcodes
- Configurable RAM size, screen size, and video memory start
- Prints a video memory region to the console
- Can load Z80 binaries

---

## Registers
| Register | Description |
|----------|-------------|
| A        | Accumulator |
| B        | General purpose |
| C        | General purpose |
| D        | General purpose |
| E        | General purpose |
| F        | Flags (not used) |
| H        | High byte of HL |
| L        | Low byte of HL |
| SP       | Stack Pointer |
| PC       | Program Counter |
| CR       | Control Register (unused) |

---

## Opcodes
ChipEmu80 supports the following opcodes:

### NOP
- **Opcode:** `0x00`
- **Description:** No operation. Increments PC by 1.

### INC / DEC HL
| Opcode | Operation |
|--------|-----------|
| 0x23   | INC HL    |
| 0x2B   | DEC HL    |

### LD r,n (Load immediate)
| Opcode | Operation |
|--------|-----------|
| 0x3E   | LD A,n    |
| 0x06   | LD B,n    |
| 0x0E   | LD C,n    |
| 0x16   | LD D,n    |
| 0x1E   | LD E,n    |
| 0x26   | LD H,n    |
| 0x2E   | LD L,n    |

### LD (HL),A / LD A,(HL)
| Opcode | Operation |
|--------|-----------|
| 0x77   | LD (HL),A |
| 0x7E   | LD A,(HL) |

### ADD A,r
| Opcode | Operation |
|--------|-----------|
| 0x7F   | ADD A,A   |
| 0x78   | ADD A,B   |
| 0x79   | ADD A,C   |
| 0x7A   | ADD A,D   |
| 0x7B   | ADD A,E   |
| 0x7C   | ADD A,H   |
| 0x7D   | ADD A,L   |
| 0x80   | ADD A,B   |
| 0x81   | ADD A,C   |
| 0x82   | ADD A,D   |
| 0x83   | ADD A,E   |
| 0x84   | ADD A,H   |
| 0x85   | ADD A,L   |
| 0x86   | ADD A,(HL)|

### JP nn
- **Opcode:** `0xC3`
- **Description:** Jump to absolute address nn (PC = nn)
- **Example:** `C3 00 10` → Jump to address 0x1000

### HALT
- **Opcode:** `0x76`
- **Description:** Stop execution and print screen and registers

---

## Configuration File
You can pass a configuration file using the `-config <file>` option.

Example `config.txt`:
```
RamSize 65536
VideoStart=C000
ScreenWidth=16
ScreenHeight=16
```

---

## Running a Binary
```bash
./chipemu80 program.bin -config config.txt
```

---

## Example Program: Print "HI"
The following binary prints "HI" at the start of video memory.
```
3E48 26C0 2E00 77 3E49 26C0 2E01 77 76
```
Explanation:
- `3E48` → LD A,'H'
- `26C0` → LD H,0xC0
- `2E00` → LD L,0x00
- `77`   → LD (HL),A
- `3E49` → LD A,'I'
- `26C0` → LD H,0xC0
- `2E01` → LD L,0x01
- `77`   → LD (HL),A
- `76`   → HALT

---

## Notes
- Only the listed opcodes are supported.
- Video memory is printed to the console every 20 ticks.
- F register is not used in this version.

---

## License
MIT

