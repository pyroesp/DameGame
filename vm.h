#ifndef _VM_H
#define _VM_H


#include <stdint.h>
#include "rom.h"
#include "ram.h"
#include "memory.h"
#include "memory_map.h"
#include "cpu.h"

typedef struct{
	Memory *BIOS;
	Memory *ROM;
	Memory *VRAM;
	Memory *RAM;
	Memory *Internal_RAM;
	Cpu *cpu;
}VM;

VM* vm_Init(void);
void vm_Free(VM *pVm);

#endif
