#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "rom.h"
#include "ram.h"
#include "cpu.h"

int main(int argc, char *argv[]){
	Memory *ROM = NULL;
	Memory *RAM = NULL;
	Memory *Internal_RAM = NULL;
	Memory *Display_RAM = NULL;

	ROM = mem_Init(ROM_SIZE, ROM_BANK_SIZE, ROM_SIZE / ROM_BANK_SIZE);
	RAM = mem_Init(RAM_SIZE, RAM_BANK_SIZE, RAM_SIZE / RAM_BANK_SIZE);
	Internal_RAM = mem_Init(MEM_RAM_INTERNAL_SIZE, 1, MEM_RAM_INTERNAL_SIZE);
	Display_RAM = mem_Init(MEM_VIDEO_RAM_SIZE, 1, MEM_VIDEO_RAM_SIZE);

	ROM->data[0] = 0x00; // NOP
	ROM->data[1] = 0xCB;
	ROM->data[2] = 0x88; // clear bit 1 of reg B
	ROM->data[3] = 0xCB;
	ROM->data[4] = 0x80; // clear bit 0 of reg B
	ROM->data[5] = 0xCB;
	ROM->data[6] = 0x31; // swap reg C

	Cpu *cpu = NULL;
	cpu = cpu_Init(&Internal_RAM->data[MEM_IO_PORTS_OFFSET - MEM_RAM_INTERNAL_OFFSET]);

	cpu->B = 0xFF;
	cpu->C = 0xAB;
	cpu_Execute_Opcode(cpu, ROM->data); // NOP
	cpu_Execute_Opcode(cpu, ROM->data); // clear bit 1 of reg B
	cpu_Execute_Opcode(cpu, ROM->data); // clear bit 0 of reg B
	cpu_Execute_Opcode(cpu, ROM->data); // swap C

	printf("cpu->B = #%02X\n", cpu->B); // result should be 0xFD
	// Checking if double register have the correct endianness (reg B = MSB, reg C = LSB)
	printf("cpu->BC = #%04X\n", cpu->BC);
	// Checking if reg array is correctly aligned with the registers
	printf("cpu->reg[B] = #%02X\n", cpu->reg[0]->R);
	printf("cpu->reg[C] = #%02X\n", cpu->reg[1]->R);
	// Checking bits are correct
	printf("cpu->reg[B].0 = #%02X\n", cpu->reg[0]->R_bits.bit_0);
	printf("cpu->reg[B].1 = #%02X\n", cpu->reg[0]->R_bits.bit_1);
	printf("cpu->reg[B].2 = #%02X\n", cpu->reg[0]->R_bits.bit_2);
	printf("cpu->reg[B].3 = #%02X\n", cpu->reg[0]->R_bits.bit_3);
	printf("cpu->reg[C].0 = #%02X\n", cpu->reg[1]->R_bits.bit_0);
	printf("cpu->reg[C].1 = #%02X\n", cpu->reg[1]->R_bits.bit_1);
	printf("cpu->reg[C].2 = #%02X\n", cpu->reg[1]->R_bits.bit_2);
	printf("cpu->reg[C].3 = #%02X\n", cpu->reg[1]->R_bits.bit_3);

	// Checking if Special register and corresponding bits are correct
	cpu->sfr->NR_50_bits.S01_volume = 0x4;
	cpu->sfr->NR_50_bits.S02_volume = 0x1;

	cpu_Free(cpu);
	cpu = NULL;
	return 0;
}
