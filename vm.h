#ifndef _VM_H
#define _VM_H

#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "debug.h"
#include "rom.h"
#include "ram.h"
#include "memory.h"
#include "memory_map.h"
#include "lcd.h"
#include "cpu.h"

// Virtual Machine structure
typedef struct{
	SDL_Window *w;
	SDL_Surface *ws;
	SDL_Event ev;
	uint32_t keys;
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
int8_t vm_LoadBios(VM *pVm, char *path);
// Run VM
int8_t vm_Run(VM *pVm);
// Read keys
void vm_ReadKeys(VM *pVm);
// Quit vm
void vm_Quit(VM *pVm);

#endif
