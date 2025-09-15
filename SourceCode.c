#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void PrintScreen();
void stackPush(unsigned short ret_addr);
unsigned short stackPop();

typedef struct {
    unsigned char A, B, C, D, E, H, L;  // registers
    unsigned short SP, PC, CR;

    unsigned char F_Z : 1;
    unsigned char F_S : 1;
    unsigned char F_P : 1;
    unsigned char F_C : 1;
    unsigned char F_H : 1;
    unsigned char F_N : 1;
} CPU;

CPU cpu;

unsigned short getHL() { return (cpu.H << 8) | cpu.L; }
unsigned short getDE() { return (cpu.D << 8) | cpu.E; }

void cleanFlags() {
    cpu.F_Z = cpu.F_S = cpu.F_P = cpu.F_C = cpu.F_H = cpu.F_N = 0;
}

void setFlagsAdd(unsigned char a, unsigned char b, unsigned char res) {
    cleanFlags();
    cpu.F_Z = (res == 0);
    cpu.F_S = (res & 0x80) != 0;
    cpu.F_H = ((a & 0xF) + (b & 0xF)) > 0xF;
    cpu.F_C = (res < a);
    cpu.F_N = 0;
}

void setFlagsSub(unsigned char a, unsigned char b, unsigned char res) {
    cleanFlags();
    cpu.F_Z = (res == 0);
    cpu.F_S = (res & 0x80) != 0;
    cpu.F_H = (a & 0xF) < (b & 0xF);
    cpu.F_C = (a < b);
    cpu.F_N = 1;
}

void CP(unsigned char val) {
    unsigned char res = cpu.A - val;
    setFlagsSub(cpu.A, val, res);
}

void setHL(unsigned short val) { cpu.H = (val >> 8) & 0xFF; cpu.L = val & 0xFF; }
void setDE(unsigned short val) { cpu.D = (val >> 8) & 0xFF; cpu.E = val & 0xFF; }

int RamSize = 65536;
int Video_Start = 0x8000;
int ScreenW = 16;
int ScreenH = 16;
unsigned char *RAM;

void loadConfig(char *entry) {
    FILE *conf = fopen(entry, "r");
    if(!conf) { perror("Error>>"); exit(1); }
    fscanf(conf, "RamSize %d\n", &RamSize);
    fscanf(conf, "VideoStart %x\n", &Video_Start);
    fscanf(conf, "ScreenWidth %d\n", &ScreenW);
    fscanf(conf, "ScreenHeight %d\n", &ScreenH);
    fclose(conf);
}

int main(int argc, char *argv[]) {
    for(int arg = 2; arg < argc; arg++) if(strcmp(argv[arg], "-config") == 0) loadConfig(argv[arg+1]);

    RAM = malloc(RamSize);
    cpu.SP = 0xFFFE;
    if(!RAM) { printf("Error>> Unable to allocate virtual ram."); exit(1); }
    memset(RAM, 0, RamSize);

    if(argc < 2) { printf("Usage: %s <binary_file>\n", argv[0]); return 1; }

    FILE *bin = fopen(argv[1], "rb");
    if(!bin) { perror("Error>>"); exit(1); }
    fread(RAM, 1, RamSize, bin);
    fclose(bin);

    int tick = 0;

    while(1) {
        unsigned char opcode = RAM[cpu.PC];

        switch(opcode) {
            case 0x00: cpu.PC++; break; // NOP

            case 0x21: { cpu.L = RAM[cpu.PC+1]; cpu.H = RAM[cpu.PC+2]; cpu.PC+=3; break; }
            case 0x11: { cpu.E = RAM[cpu.PC+1]; cpu.D = RAM[cpu.PC+2]; cpu.PC+=3; break; }

            case 0x23: setHL(getHL()+1); cpu.PC++; break;
            case 0x13: setDE(getDE()+1); cpu.PC++; break;
            case 0x2B: setHL(getHL()-1); cpu.PC++; break;

            case 0x3E: cpu.A = RAM[cpu.PC+1]; cpu.PC+=2; break;
            case 0x06: cpu.B = RAM[cpu.PC+1]; cpu.PC+=2; break;
            case 0x0E: cpu.C = RAM[cpu.PC+1]; cpu.PC+=2; break;
            case 0x16: cpu.D = RAM[cpu.PC+1]; cpu.PC+=2; break;
            case 0x1E: cpu.E = RAM[cpu.PC+1]; cpu.PC+=2; break;
            case 0x26: cpu.H = RAM[cpu.PC+1]; cpu.PC+=2; break;
            case 0x2E: cpu.L = RAM[cpu.PC+1]; cpu.PC+=2; break;

            case 0x77: RAM[getHL()] = cpu.A; cpu.PC++; break;
            case 0x12: RAM[getDE()] = cpu.A; cpu.PC++; break;
            case 0x7E: cpu.A = RAM[getHL()]; cpu.PC++; break;
            case 0x1A: cpu.A = RAM[getDE()]; cpu.PC++; break;

            // ADD A,r completo do manual
            case 0x7F: setFlagsAdd(cpu.A, cpu.A, cpu.A+cpu.A); cpu.A += cpu.A; cpu.PC++; break;
            case 0x78: { unsigned char res = cpu.A+cpu.B; setFlagsAdd(cpu.A, cpu.B, res); cpu.A=res; cpu.PC++; break; }
            case 0x79: { unsigned char res = cpu.A+cpu.C; setFlagsAdd(cpu.A, cpu.C, res); cpu.A=res; cpu.PC++; break; }
            case 0x7A: { unsigned char res = cpu.A+cpu.D; setFlagsAdd(cpu.A, cpu.D, res); cpu.A=res; cpu.PC++; break; }
            case 0x7B: { unsigned char res = cpu.A+cpu.E; setFlagsAdd(cpu.A, cpu.E, res); cpu.A=res; cpu.PC++; break; }
            case 0x7C: { unsigned char res = cpu.A+cpu.H; setFlagsAdd(cpu.A, cpu.H, res); cpu.A=res; cpu.PC++; break; }
            case 0x7D: { unsigned char res = cpu.A+cpu.L; setFlagsAdd(cpu.A, cpu.L, res); cpu.A=res; cpu.PC++; break; }
            case 0x86: { unsigned char res = cpu.A+RAM[getHL()]; setFlagsAdd(cpu.A, RAM[getHL()], res); cpu.A=res; cpu.PC++; break; }

            // CP A,r com flags
            case 0xB8: { unsigned char res = cpu.A-cpu.B; setFlagsSub(cpu.A, cpu.B, res); cpu.PC++; break; }
            case 0xFE: { unsigned char val=RAM[cpu.PC+1]; unsigned char res=cpu.A-val; setFlagsSub(cpu.A,val,res); cpu.PC+=2; break; }

            case 0xC0: if(!cpu.F_Z) cpu.PC=stackPop(); else cpu.PC++; break;
            case 0xC3: cpu.PC = (RAM[cpu.PC+2]<<8)|RAM[cpu.PC+1]; break;
            case 0xCA: { unsigned short addr=(RAM[cpu.PC+2]<<8)|RAM[cpu.PC+1]; if(cpu.F_Z) cpu.PC=addr; else cpu.PC+=3; break; }
            case 0xCD: { unsigned short addr=(RAM[cpu.PC+2]<<8)|RAM[cpu.PC+1]; stackPush(cpu.PC+3); cpu.PC=addr; break; }
            case 0xC8: if(cpu.F_Z) cpu.PC=stackPop(); else cpu.PC++; break;
            case 0xC9: cpu.PC=stackPop(); break;

            case 0x76: // HALT
                PrintScreen();
                printf("> Exit registers \n");
                printf("A-%02x B-%02x C-%02x D-%02x E-%02x H-%02x L-%02x\n",
                       cpu.A, cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L);
                free(RAM);
                return 0;

            default: printf("Error>> Unknown opcode %02X at %04X\n", opcode, cpu.PC); cpu.PC++; break;
        }

        if(tick % 20 == 0) { PrintScreen(); tick=0; }
        tick++;
    }

    return 0;
}

void PrintScreen() {
    printf("\033[H\033[J");
    for(int y=0;y<ScreenH;y++) {
        for(int x=0;x<ScreenW;x++) {
            printf("\033[44;33m\033[22m");
            unsigned char ch = RAM[Video_Start + y*ScreenW + x];
            putchar(ch ? ch : ' ');
        }
        printf("\033[0m\n");
    }
}

void stackPush(unsigned short ret_addr) {
    RAM[cpu.SP--] = ret_addr & 0xFF;
    RAM[cpu.SP--] = (ret_addr>>8) & 0xFF;
}

unsigned short stackPop() {
    unsigned char high=RAM[++cpu.SP];
    unsigned char low=RAM[++cpu.SP];
    return (high<<8)|low;
}
