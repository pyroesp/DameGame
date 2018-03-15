#include <stdint.h>
#include <stdio.h>

#include "cpu.h"

int main(int argc, char *argv[]){
    int i;
	uint8_t mem[MEM_TOTAL] = {0};

	mem[1] = 0xCB;
	mem[2] = 0x88; // clear bit 1 of reg B
	mem[3] = 0xCB;
	mem[4] = 0x80; // clear bit 0 of reg B

	Cpu *cpu = NULL;
	cpu = cpu_Init(mem);

	for (i = 0; i < 5; i++)
        printf("#%02X  ", mem[i]);
    printf("\n");

    cpu->B = 0xFF;
	cpu_Execute_Opcode(cpu, mem);
	cpu_Execute_Opcode(cpu, mem);
	cpu_Execute_Opcode(cpu, mem);

	printf("cpu->B = #%02X\n", cpu->B); // result should be 0xFD
	// Checking if double register have the correct endianess (reg B = MSB, reg C = LSB)
	printf("cpu->BC = #%04X\n", cpu->BC);

    // Checking if Special register and corresponding bits are correct
	cpu->sfr->NR_50_bits.S01_volume = 0x4;
	cpu->sfr->NR_50_bits.S02_output = 1;
	cpu->sfr->NR_50_bits.S02_volume = 0x1;

	printf("NR_50 = #%02X | address = $%04X\n", cpu->sfr->NR_50, &cpu->sfr->NR_50 - mem);

	cpu_Free(cpu);
	cpu = NULL;
	return 0;
}
