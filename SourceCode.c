#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void PrintScreen();

typedef struct {
    unsigned char A, B, C, D, E, F, H, L;
    unsigned short SP, PC, CR;
} CPU;

CPU cpu;

unsigned short getHL() {
    return (cpu.H << 8) | cpu.L;
}

void setHL(unsigned short val) {
    cpu.H = (val >> 8) & 0xFF;
    cpu.L = val & 0xFF;
}

int RamSize = 65536;
int Video_Start = 0x8000;
int ScreenW = 16;
int ScreenH = 16;

unsigned char *RAM;

void loadConfig(char *entry) {
	FILE *conf = fopen(entry, "r");
	if(conf == NULL) {
		perror("Error>>");
		exit(1);
	}

	fscanf(conf, "RamSize %d\n", &RamSize);
	fscanf(conf, "VideoStart %x\n", &Video_Start);
	fscanf(conf, "ScreenWidth %d\n", &ScreenW);
	fscanf(conf, "ScreenHeight %d\n", &ScreenH);
	
	fclose(conf);
}

int main(int argc, char *argv[]) {

	for(int arg = 2; arg < argc; arg++) {
		if(strcmp(argv[arg], "-config") == 0) {
			loadConfig(argv[arg+1]);
		}
	}

	RAM = malloc(RamSize);
	if(!RAM) {
		printf("Error>> Unable to allocate virtual ram.");
		exit(1);
	}
	memset(RAM, 0, RamSize);

    if(argc < 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    FILE *bin = fopen(argv[1], "rb");
    if(bin == NULL) {
        perror("Error>>");
        exit(1);
    }

    size_t read_bin = fread(RAM, 1, RamSize, bin);
    fclose(bin);

    int tick = 0;

    while(1) {
        unsigned char opcode = RAM[cpu.PC];

        switch(opcode) {

            case 0x00: // NOP
                cpu.PC++;
                break;

            case 0x23: // INC HL
                setHL(getHL() + 1);
                cpu.PC++;
                break;

            case 0x2B: // DEC HL
                setHL(getHL() - 1);
                cpu.PC++;
                break;

            case 0x3E: // LD A,n
                cpu.A = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x06: // LD B,n
                cpu.B = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x0E: // LD C,n
                cpu.C = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x16: // LD D,n
                cpu.D = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x1E: // LD E,n
                cpu.E = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x26: // LD H,n
                cpu.H = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x2E: // LD L,n
                cpu.L = RAM[cpu.PC + 1];
                cpu.PC += 2;
                break;

            case 0x77: // LD (HL),A
                RAM[getHL()] = cpu.A;
                cpu.PC++;
                break;

            case 0x7E: // LD A,(HL)
                cpu.A = RAM[getHL()];
                cpu.PC++;
                break;

            // ADD A,r
            case 0x80: cpu.A += cpu.B; cpu.PC++; break;
            case 0x81: cpu.A += cpu.C; cpu.PC++; break;
            case 0x82: cpu.A += cpu.D; cpu.PC++; break;
            case 0x83: cpu.A += cpu.E; cpu.PC++; break;
            case 0x84: cpu.A += cpu.H; cpu.PC++; break;
            case 0x85: cpu.A += cpu.L; cpu.PC++; break;
            case 0x86: cpu.A += RAM[getHL()]; cpu.PC++; break;
            case 0x7F: cpu.A += cpu.A; cpu.PC++; break;
            case 0x78: cpu.A += cpu.B; cpu.PC++; break;
            case 0x79: cpu.A += cpu.C; cpu.PC++; break;
            case 0x7A: cpu.A += cpu.D; cpu.PC++; break;
            case 0x7B: cpu.A += cpu.E; cpu.PC++; break;
            case 0x7C: cpu.A += cpu.H; cpu.PC++; break;
            case 0x7D: cpu.A += cpu.L; cpu.PC++; break;

            case 0xC3: { // JP nn
                unsigned char low = RAM[cpu.PC + 1];
                unsigned char high = RAM[cpu.PC + 2];
                cpu.PC = (high << 8) | low;
                break;
            }

            case 0x76: // HALT
                PrintScreen();
                printf("> Exit registers \n");
                printf("A-%02x B-%02x C-%02x D-%02x E-%02x F-%02x H-%02x L-%02x\n",
                       cpu.A, cpu.B, cpu.C, cpu.D, cpu.E, cpu.F, cpu.H, cpu.L);
                free(RAM);
                return 0;

            default:
                printf("Error>> Unknown opcode %02X at %04X\n", opcode, cpu.PC);
                cpu.PC++;
                break;
        }

        if(tick % 20 == 0) {
            PrintScreen();
            tick = 0;
        }

        tick++;
    }

    return 0;
}

void PrintScreen() {
	printf("\033[H\033[J");
    for (int y = 0; y < ScreenH; y++) {
        for (int x = 0; x < ScreenW; x++) {
            unsigned char ch = RAM[Video_Start + y * ScreenW + x];
            if(ch == 0x00) {
            	putchar(' ');
            } else putchar(ch);
        }
        putchar('\n');
    }
}
