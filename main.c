#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "rom.h"
#include "ram.h"
#include "cpu.h"

int main(int argc, char *argv[]){
	Memory *BIOS = NULL;
	Memory *ROM = NULL;
	Memory *RAM = NULL;
	Memory *Internal_RAM = NULL;
	Memory *Display_RAM = NULL;

	MemoryMap cpu_map[MEM_ADDRESS_SPACES];

	BIOS = mem_Init(MEM_ROM_BIOS_SIZE, 1, MEM_ROM_BIOS_SIZE);
	ROM = mem_Init(ROM_SIZE, ROM_BANK_SIZE, ROM_SIZE / ROM_BANK_SIZE);
	RAM = mem_Init(RAM_SIZE, RAM_BANK_SIZE, RAM_SIZE / RAM_BANK_SIZE);
	Internal_RAM = mem_Init(MEM_RAM_INTERNAL_SIZE, 1, MEM_RAM_INTERNAL_SIZE);
	Display_RAM = mem_Init(MEM_VIDEO_RAM_SIZE, 1, MEM_VIDEO_RAM_SIZE);


	// set up BIOS
	mem_CopyInfo(&cpu_map[MAP_ROM_BIOS].mem, BIOS);
	cpu_map[MAP_ROM_BIOS].map_offset = MEM_ROM_BIOS_OFFSET;

	// set up ROM
	mem_CopyInfo(&cpu_map[MAP_ROM_BANK_0].mem, ROM);
	cpu_map[MAP_ROM_BANK_0].map_offset = MEM_ROM_BANK_0_OFFSET;

	// set up ROM switch bank
	mem_CopyInfo(&cpu_map[MAP_ROM_BANK_SWITCH].mem, ROM);
	mem_SetStartIndex(&cpu_map[MAP_ROM_BANK_SWITCH].mem, cpu_map[MAP_ROM_BANK_SWITCH].mem.bank_size);
	cpu_map[MAP_ROM_BANK_SWITCH].map_offset = MEM_ROM_SWITCH_BANK_OFFSET;

	// set up Video RAM
	mem_CopyInfo(&cpu_map[MAP_VRAM].mem, Display_RAM);
	cpu_map[MAP_VRAM].map_offset = MEM_RAM_VIDEO_OFFSET;

	// set up RAM switch bank
	mem_CopyInfo(&cpu_map[MAP_RAM_BANK_SWITCH].mem, RAM);
	cpu_map[MAP_RAM_BANK_SWITCH].map_offset = MEM_RAM_SWITCH_OFFSET;

	// set up internal RAM
	cpu_map[MAP_RAM_INTERNAL].mem.data = Internal_RAM->data;
	cpu_map[MAP_RAM_INTERNAL].mem.banks = 1;
	cpu_map[MAP_RAM_INTERNAL].mem.size = RAM_BANK_SIZE;
	cpu_map[MAP_RAM_INTERNAL].mem.bank_size = RAM_BANK_SIZE;
	cpu_map[MAP_RAM_INTERNAL].mem.start_idx = 0;
	cpu_map[MAP_RAM_INTERNAL].map_offset = MEM_RAM_INTERNAL_OFFSET;

	// set up echo of internal RAM
	cpu_map[MAP_RAM_INTERNAL_ECHO].mem.data = Internal_RAM->data;
	cpu_map[MAP_RAM_INTERNAL_ECHO].mem.banks = 1;
	cpu_map[MAP_RAM_INTERNAL_ECHO].mem.size = MEM_RAM_INTERNAL_ECHO_SIZE;
	cpu_map[MAP_RAM_INTERNAL_ECHO].mem.bank_size = MEM_RAM_INTERNAL_ECHO_SIZE;
	cpu_map[MAP_RAM_INTERNAL_ECHO].mem.start_idx = 0;
	cpu_map[MAP_RAM_INTERNAL_ECHO].map_offset = MEM_RAM_INTERNAL_ECHO_OFFSET;

	// set up object attribute mem
	cpu_map[MAP_OAM].mem.data = &Internal_RAM->data[MEM_SPRITE_ATTRI_OFFSET - MEM_RAM_INTERNAL_OFFSET];
	cpu_map[MAP_OAM].mem.banks = 1;
	cpu_map[MAP_OAM].mem.size = MEM_SPRITE_ATTRI_SIZE;
	cpu_map[MAP_OAM].mem.bank_size = MEM_SPRITE_ATTRI_SIZE;
	cpu_map[MAP_OAM].mem.start_idx = 0;
	cpu_map[MAP_OAM].map_offset = MEM_SPRITE_ATTRI_OFFSET;

	// set up unusable section of map, not really needed
	cpu_map[MAP_UNUSABLE].mem.data = &Internal_RAM->data[MEM_UNUSABLE_OFFSET - MEM_RAM_INTERNAL_OFFSET];
	cpu_map[MAP_UNUSABLE].mem.banks = 1;
	cpu_map[MAP_UNUSABLE].mem.size = MEM_UNUSABLE_SIZE;
	cpu_map[MAP_UNUSABLE].mem.bank_size = MEM_UNUSABLE_SIZE;
	cpu_map[MAP_UNUSABLE].mem.start_idx = 0;
	cpu_map[MAP_UNUSABLE].map_offset = MEM_UNUSABLE_OFFSET;

	// set up IO ports, including interrupt enable register
	cpu_map[MAP_IO_PORTS].mem.data = &Internal_RAM->data[MEM_IO_PORTS_OFFSET - MEM_RAM_INTERNAL_OFFSET];
	cpu_map[MAP_IO_PORTS].mem.banks = 1;
	cpu_map[MAP_IO_PORTS].mem.size = MEM_IO_PORTS_SIZE;
	cpu_map[MAP_IO_PORTS].mem.bank_size = MEM_IO_PORTS_SIZE;
	cpu_map[MAP_IO_PORTS].mem.start_idx = 0;
	cpu_map[MAP_IO_PORTS].map_offset = MEM_IO_PORTS_OFFSET;


	ROM->data[0] = 0x00; // NOP
	ROM->data[1] = 0xCB;
	ROM->data[2] = 0x88; // clear bit 1 of reg B
	ROM->data[3] = 0xCB;
	ROM->data[4] = 0x80; // clear bit 0 of reg B
	ROM->data[5] = 0xCB;
	ROM->data[6] = 0x31; // swap reg C

	Cpu *cpu = NULL;
	cpu = cpu_Init(cpu_map[MAP_IO_PORTS].mem.data);

	cpu->B = 0xFF;
	cpu->C = 0xAB;
	cpu_ExecuteOpcode(cpu, ROM->data); // NOP
	cpu_ExecuteOpcode(cpu, ROM->data); // clear bit 1 of reg B
	cpu_ExecuteOpcode(cpu, ROM->data); // clear bit 0 of reg B
	cpu_ExecuteOpcode(cpu, ROM->data); // swap C

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
