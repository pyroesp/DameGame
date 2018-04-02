#ifndef _VM_H
#define _VM_H

#include <stdint.h>
#include <stdio.h>
#include "debug.h"
#include "rom.h"
#include "ram.h"
#include "memory.h"
#include "memory_map.h"
#include "cpu.h"

// Virtual Machine structure
typedef struct{
	Memory *BIOS;
	Memory *ROM;
	Memory *VRAM;
	Memory *RAM;
	Memory *Internal_RAM;
	Cpu *cpu;
}VM;

// Initialize and return a VM structure
VM* vm_Init(void);
// Load bios to VM
int vm_LoadBios(VM *pVm, char *path);
// Free bios
void vm_Free(VM *pVm);

#endif
