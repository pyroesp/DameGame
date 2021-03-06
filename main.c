#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "vm.h"

int main(int argc, char *argv[]){
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
	if (!vm)
		return -1;

	if (vm_LoadBios(vm, "bios/bios.gb") != 0){
		// TODO: Setup cpu and memory as if the bios just executed
		return -1;
	}

	// Write logo to ROM at correct location for the bios to check
	if (mem_WriteMulti(vm->ROM, 0x104, test_logo, 48) == 0x100)
		return -1;

	// Run bios
	vm_Run(vm);

	// Exit
	DEBUG_PRINTF("\nFree stuff & exit\n");
	vm_Quit(vm);
	return 0;
}
