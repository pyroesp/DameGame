#include <stdint.h>
#include <stdio.h>

#include "cpu.h"

int main(int argc, char *argv[]){
    int i;
	uint8_t mem[MEM_TOTAL] = {0};

    mem[0] = 0x00; // NOP
	mem[1] = 0xCB;
	mem[2] = 0x88; // clear bit 1 of reg B
	mem[3] = 0xCB;
	mem[4] = 0x80; // clear bit 0 of reg B
	mem[5] = 0xCB;
	mem[6] = 0x31; // swap reg C

	Cpu *cpu = NULL;
	cpu = cpu_Init(mem);

    cpu->B = 0xFF;
    cpu->C = 0xAB;
	cpu_Execute_Opcode(cpu, mem); // NOP
	cpu_Execute_Opcode(cpu, mem); // clear bit 1 of reg B
	cpu_Execute_Opcode(cpu, mem); // clear bit 0 of reg B
	cpu_Execute_Opcode(cpu, mem); // swap C

	printf("cpu->B = #%02X\n", cpu->B); // result should be 0xFD
	// Checking if double register have the correct endianness (reg B = MSB, reg C = LSB)
	printf("cpu->BC = #%04X\n", cpu->BC);
	// Checking if reg array is correctly aligned with the registers
	printf("cpu->reg[B] = #%02X\n", cpu->reg[0]->R);
	printf("cpu->reg[C] = #%02X\n", cpu->reg[1]->R);

    // Checking if Special register and corresponding bits are correct
	cpu->sfr->NR_50_bits.S01_volume = 0x4;
	cpu->sfr->NR_50_bits.S02_volume = 0x1;

	printf("NR_50 = #%02X | address = $%04X\n", cpu->sfr->NR_50, &cpu->sfr->NR_50 - mem);

	cpu_Free(cpu);
	cpu = NULL;
	return 0;
}
