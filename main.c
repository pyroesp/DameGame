#include <stdint.h>
#include <stdio.h>

#include "vm.h"

int main(int argc, char *argv[]){
	VM *vm = NULL;
	vm = vm_Init();

	vm->cpu->sfr->BIOS = 1; // disable BIOS for now

    // Write some opcodes into ROM
	vm->ROM->data[0] = 0x00; // NOP
	vm->ROM->data[1] = 0xCB;
	vm->ROM->data[2] = 0x88; // clear bit 1 of reg B
	vm->ROM->data[3] = 0xCB;
	vm->ROM->data[4] = 0x80; // clear bit 0 of reg B
	vm->ROM->data[5] = 0xCB;
	vm->ROM->data[6] = 0x31; // swap reg C

	vm->cpu->B = 0xFF;
	vm->cpu->C = 0xAB;

	printf("cpu->BC = #%04X\n", vm->cpu->BC); // result should be 0xFFAB

	cpu_Run(vm->cpu); // NOP
	cpu_Run(vm->cpu); // clear bit 1 of reg B
	cpu_Run(vm->cpu); // clear bit 0 of reg B
	cpu_Run(vm->cpu); // swap C

	printf("cpu->B = #%02X\n", vm->cpu->B); // result should be 0xFD
	// Checking if double register have the correct endianness (reg B = MSB, reg C = LSB)
	printf("cpu->BC = #%04X\n", vm->cpu->BC); // result should be 0xBA
	// Checking if reg array is correctly aligned with the registers
	printf("cpu->reg[B] = #%02X\n", vm->cpu->reg[0]->R);
	printf("cpu->reg[C] = #%02X\n", vm->cpu->reg[1]->R);
	// Checking bits are correct
	printf("cpu->reg[B].0 = #%02X\n", vm->cpu->reg[0]->R_bits.bit_0);
	printf("cpu->reg[B].1 = #%02X\n", vm->cpu->reg[0]->R_bits.bit_1);
	printf("cpu->reg[B].2 = #%02X\n", vm->cpu->reg[0]->R_bits.bit_2);
	printf("cpu->reg[B].3 = #%02X\n", vm->cpu->reg[0]->R_bits.bit_3);
	printf("cpu->reg[C].0 = #%02X\n", vm->cpu->reg[1]->R_bits.bit_0);
	printf("cpu->reg[C].1 = #%02X\n", vm->cpu->reg[1]->R_bits.bit_1);
	printf("cpu->reg[C].2 = #%02X\n", vm->cpu->reg[1]->R_bits.bit_2);
	printf("cpu->reg[C].3 = #%02X\n", vm->cpu->reg[1]->R_bits.bit_3);

	// Checking if Special register and corresponding bits are correct
	vm->cpu->sfr->NR_50_bits.S01_volume = 0x4;
	vm->cpu->sfr->NR_50_bits.S02_volume = 0x1;
	printf("cpu->sfr->NR_50_bits.S01_volume = #%02X\n", vm->cpu->sfr->NR_50_bits.S01_volume);
	printf("cpu->sfr->NR_50_bits.S02_volume = #%02X\n", vm->cpu->sfr->NR_50_bits.S02_volume);

    printf("Free stuff & exit\n");
	vm_Free(vm);
	return 0;
}
