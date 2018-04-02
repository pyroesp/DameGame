#include <stdint.h>
#include <stdio.h>

#include "vm.h"

int main(int argc, char *argv[]){
	int i;
	uint8_t test_logo[48] = { // Nintendo Logo
		0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b,
		0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d,
		0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
		0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99,
		0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc,
		0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e
	};
	VM *vm = NULL;
	vm = vm_Init();
	if (vm_LoadBios(vm, "bios/bios.gb") != 0){
		// TODO: Setup cpu and memory as if the bios just executed
		return -1;
	}

	// Test if the bios checks the logo correctly
	if (mem_WriteMulti(vm->ROM, 0x104, test_logo, 48) == 0x100)
		return -1;

	// Display bios memory
	for (i = 1; i < MEM_ROM_BIOS_SIZE + 1; i++){
		DEBUG_PRINTF("%02X ", vm->BIOS->data[i - 1]);
		if (i % 16 == 0)
			DEBUG_PRINTF("\n");
	}

	// Run until bios is disabled -> gets stuck on a wait for vertical blank loop
	for(i = 0; vm->cpu->sfr->BIOS == 0; i++){
		cpu_Run(vm->cpu);
	}

	DEBUG_PRINTF("\nFree stuff & exit\n");
	vm_Free(vm);
	return 0;
}
