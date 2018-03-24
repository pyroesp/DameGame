#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include <stdlib.h>
#include <stdint.h>
#include "memory.h"
#include "rom.h"
#include "ram.h"

#define MEM_TOTAL_SIZE (0x10000)

#define MEM_ADDRESS_SPACES (10)

enum{
	MAP_ROM_BIOS, // 0000 - 00FF
	MAP_ROM_BANK_0, // 0000 - 3FFF
	MAP_ROM_BANK_SWITCH, // 4000 - 7FFF
	MAP_VRAM, // 8000 - 9FFF
	MAP_RAM_BANK_SWITCH, // A000 - BFFF
	MAP_RAM_INTERNAL, // C000 - DFFF
	MAP_RAM_INTERNAL_ECHO, // E000 - FDFF
	MAP_OAM, // FE00 - FE9F
	MAP_UNUSABLE, // FEA0 - FE9F
	MAP_IO_PORTS, // FF00 - FFFF
};

#define MEM_ROM_BIOS_SIZE (0x100)
#define MEM_VIDEO_RAM_SIZE (0x2000)
#define MEM_RAM_INTERNAL_SIZE (0x4000) // internal_ram -> [C000; FFFF]
#define MEM_RAM_INTERNAL_ECHO_SIZE (0x1E00)
#define MEM_SPRITE_ATTRI_SIZE (0x00A0) // OAM
#define MEM_UNUSABLE_SIZE (0x0060)
#define MEM_IO_PORTS_SIZE (0x0100)

#define MEM_ROM_BIOS_OFFSET (0x0000)
#define MEM_ROM_BANK_0_OFFSET (0x0000)
#define MEM_ROM_SWITCH_BANK_OFFSET (0x4000)
#define MEM_RAM_VIDEO_OFFSET (0x8000)
#define MEM_RAM_SWITCH_OFFSET (0xA000)
#define MEM_RAM_INTERNAL_OFFSET (0xC000)
#define MEM_RAM_INTERNAL_ECHO_OFFSET (0xE000)
#define MEM_SPRITE_ATTRI_OFFSET (0xFE00)
#define MEM_UNUSABLE_OFFSET (0xFEA0)
#define MEM_IO_PORTS_OFFSET (0xFF00)
#define MEM_RAM_INTERNAL2_OFFSET (0xFF80)


typedef struct{
	Memory mem; // RAM, ROM, VRAM, ...
	uint16_t map_offset; // address offset of memory map
}MemoryMap;


#endif

