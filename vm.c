#include "vm.h"

VM* vm_Init(void){
	VM *vm = NULL;
	Cpu *cpu = NULL;
	Memory *BIOS = NULL;
	Memory *ROM = NULL;
	Memory *VRAM = NULL;
	Memory *RAM = NULL;
	Memory *Internal_RAM = NULL;

	BIOS = mem_Init(MEM_ROM_BIOS_SIZE, 1, MEM_ROM_BIOS_SIZE);
	ROM = mem_Init(ROM_SIZE, ROM_BANK_SIZE, ROM_SIZE / ROM_BANK_SIZE);
	VRAM = mem_Init(MEM_VIDEO_RAM_SIZE, 1, MEM_VIDEO_RAM_SIZE);
	RAM = mem_Init(RAM_SIZE, RAM_BANK_SIZE, RAM_SIZE / RAM_BANK_SIZE);
	Internal_RAM = mem_Init(MEM_RAM_INTERNAL_SIZE_TOTAL, 1, MEM_RAM_INTERNAL_SIZE_TOTAL);

	// Init CPU
	cpu = cpu_Init();

	// set up BIOS
	mem_CopyInfo(&cpu->map[MAP_ROM_BIOS].mem, BIOS);
	cpu->map[MAP_ROM_BIOS].offset = MEM_ROM_BIOS_OFFSET;

	// set up ROM
	mem_CopyInfo(&cpu->map[MAP_ROM_BANK_0].mem, ROM);
	cpu->map[MAP_ROM_BANK_0].offset = MEM_ROM_BANK_0_OFFSET;

	// set up ROM switch bank
	mem_CopyInfo(&cpu->map[MAP_ROM_BANK_SWITCH].mem, ROM);
	mem_SetStartIndex(&cpu->map[MAP_ROM_BANK_SWITCH].mem, cpu->map[MAP_ROM_BANK_SWITCH].mem.bank_size);
	cpu->map[MAP_ROM_BANK_SWITCH].offset = MEM_ROM_SWITCH_BANK_OFFSET;

	// set up Video RAM
	mem_CopyInfo(&cpu->map[MAP_VRAM].mem, VRAM);
	cpu->map[MAP_VRAM].offset = MEM_VIDEO_RAM_OFFSET;

	// set up external RAM switch bank
	mem_CopyInfo(&cpu->map[MAP_RAM_BANK_SWITCH].mem, RAM);
	cpu->map[MAP_RAM_BANK_SWITCH].offset = MEM_RAM_SWITCH_OFFSET;

	// set up internal RAM $C000 - $DFFF
	cpu->map[MAP_RAM_INTERNAL].mem.data = Internal_RAM->data;
	cpu->map[MAP_RAM_INTERNAL].mem.banks = 1;
	cpu->map[MAP_RAM_INTERNAL].mem.size = RAM_BANK_SIZE;
	cpu->map[MAP_RAM_INTERNAL].mem.bank_size = RAM_BANK_SIZE;
	cpu->map[MAP_RAM_INTERNAL].mem.start_idx = 0;
	cpu->map[MAP_RAM_INTERNAL].offset = MEM_RAM_INTERNAL_OFFSET;

	// set up echo of internal RAM $E000 - $FDFF => points to $C000 - $DFFF
	cpu->map[MAP_RAM_INTERNAL_ECHO].mem.data = Internal_RAM->data;
	cpu->map[MAP_RAM_INTERNAL_ECHO].mem.banks = 1;
	cpu->map[MAP_RAM_INTERNAL_ECHO].mem.size = MEM_RAM_INTERNAL_ECHO_SIZE;
	cpu->map[MAP_RAM_INTERNAL_ECHO].mem.bank_size = MEM_RAM_INTERNAL_ECHO_SIZE;
	cpu->map[MAP_RAM_INTERNAL_ECHO].mem.start_idx = 0;
	cpu->map[MAP_RAM_INTERNAL_ECHO].offset = MEM_RAM_INTERNAL_ECHO_OFFSET;

	// set up object attribute mem $FE00 - $FE9F
	cpu->map[MAP_OAM].mem.data = &Internal_RAM->data[MEM_SPRITE_ATTRI_OFFSET - MEM_RAM_INTERNAL_OFFSET];
	cpu->map[MAP_OAM].mem.banks = 1;
	cpu->map[MAP_OAM].mem.size = MEM_SPRITE_ATTRI_SIZE;
	cpu->map[MAP_OAM].mem.bank_size = MEM_SPRITE_ATTRI_SIZE;
	cpu->map[MAP_OAM].mem.start_idx = 0;
	cpu->map[MAP_OAM].offset = MEM_SPRITE_ATTRI_OFFSET;

	// set up HRAM section of map $FF80 - $FFFE
	cpu->map[MAP_HRAM].mem.data = &Internal_RAM->data[MEM_HRAM_OFFSET - MEM_RAM_INTERNAL_OFFSET];
	cpu->map[MAP_HRAM].mem.banks = 1;
	cpu->map[MAP_HRAM].mem.size = MEM_HRAM_SIZE;
	cpu->map[MAP_HRAM].mem.bank_size = MEM_HRAM_SIZE;
	cpu->map[MAP_HRAM].mem.start_idx = 0;
	cpu->map[MAP_HRAM].offset = MEM_HRAM_OFFSET;

	// set up IO ports, including interrupt enable register $FF00 - $FFFF
	cpu->map[MAP_IO_PORTS].mem.data = &Internal_RAM->data[MEM_IO_PORTS_OFFSET - MEM_RAM_INTERNAL_OFFSET];
	cpu->map[MAP_IO_PORTS].mem.banks = 1;
	cpu->map[MAP_IO_PORTS].mem.size = MEM_IO_PORTS_SIZE;
	cpu->map[MAP_IO_PORTS].mem.bank_size = MEM_IO_PORTS_SIZE;
	cpu->map[MAP_IO_PORTS].mem.start_idx = 0;
	cpu->map[MAP_IO_PORTS].offset = MEM_IO_PORTS_OFFSET;

    // Set SFR pointer
	cpu_SetSpecialRegisters(cpu, cpu->map[MAP_IO_PORTS].mem.data);
	cpu->sfr->BIOS = 0;
	// Set IE register
	cpu_SetInterruptEnableRegister(cpu, &Internal_RAM->data[MEM_IE_REG_OFFSET - MEM_RAM_INTERNAL_OFFSET]);

	// add all to VM
	vm = (VM*)malloc(sizeof(VM));
	vm->BIOS = BIOS;
	vm->ROM = ROM;
	vm->VRAM = VRAM;
	vm->RAM = RAM;
	vm->Internal_RAM = Internal_RAM;
	vm->cpu = cpu;

	return vm;
}

int vm_LoadBios(VM *pVm, char *path){
	uint16_t fsize = 0, bsize = 0;
	uint8_t *p;
	FILE *bios = NULL;
	bios = fopen(path, "rb");
	if (!bios)
		return -1;
	for (fsize = 0; fgetc(bios) != EOF; fsize++);
	rewind(bios);

	if (fsize != MEM_ROM_BIOS_SIZE){
		fclose(bios);
		return -2;
	}

	for (p = pVm->BIOS->data; bsize < fsize; *p = fgetc(bios), p++, bsize++);
	fclose(bios);
	return 0;
}

void vm_Free(VM *pVm){
	mem_Free(pVm->BIOS);
	mem_Free(pVm->ROM);
	mem_Free(pVm->VRAM);
	mem_Free(pVm->RAM);
	mem_Free(pVm->Internal_RAM);
	cpu_Free(pVm->cpu);
}
