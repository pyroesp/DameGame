#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include <stdlib.h>
#include <stdint.h>
#include "memory.h"
#include "rom.h"
#include "ram.h"

#define MEM_TOTAL_SIZE (0x10000)

#define MEM_ADDRESS_SPACES (10)

/*
+---------------------+---------------+------+
|         Map         | Address Range | Size |
+---------------------+---------------+------+
| BIOS                | $0000 - $00FF | 256B |
| ROM bank #0         | $0000 - $3FFF | 16kB |
| switchable ROM bank | $4000 - $7FFF | 16kB |
| Video RAM           | $8000 - 9FFF  | 8kB  |
| Switchable RAM bank | $A000 - $BFFF | 8kB  |
| Internal RAM        | $C000 - $FFFF | 16kB |
+---------------------+---------------+------+

I chose to put the OAM, IO ports and HRAM inside the internal RAM
and make the internal RAM one big memory bank

+-----------------------------------------------------------------------+
|                             Internal RAM                              |
+---------------------------+---------------+-------+-------------------+
|        Subsections        | Address Range | Size  |       Extra       |
+---------------------------+---------------+-------+-------------------+
| Internal RAM              | $C000 - $DFFF | 8kB   |                   |
| Echo of Internal RAM      | $E000 - $FDFF | 7.5kB |                   |
| Object Attribute Memory   | $FE00 - $FE9F | 160B  |                   |
| Empty Unusable            | $FEA0 - $FEFF | 96B   | Not used/mapped   |
| I/O Ports                 | $FF00 - $FF4B | 76B   |                   |
| Empty Unusable ?          | $FF4C - $FF7F | 52B   | $FF50 BIOS Enable |
| Internal High RAM         | $FF80 - $FFFE | 127B  |                   |
| Interrupt Enable Register | $FFFF         | 1B    |                   |
+---------------------------+---------------+-------+-------------------+

*Size in bytes

*/

enum{
	MAP_ROM_BIOS, // $0000 - $00FF
	MAP_ROM_BANK_0, // $0000 - $3FFF
	MAP_ROM_BANK_SWITCH, // $4000 - $7FFF
	MAP_VRAM, // $8000 - $9FFF
	MAP_RAM_BANK_SWITCH, // $A000 - $BFFF
	MAP_RAM_INTERNAL, // $C000 - $DFFF
	MAP_RAM_INTERNAL_ECHO, // $E000 - $FDFF
	MAP_OAM, // $FE00 - $FE9F
	MAP_HRAM, // $FF4C - $FFFE
	MAP_IO_PORTS, // $FF00 - $FFFF
};

#define MEM_ROM_BIOS_SIZE (0x100)
#define MEM_VIDEO_RAM_SIZE (0x2000)
#define MEM_RAM_INTERNAL_SIZE_TOTAL (0x4000) // full internal ram -> [$C000; $FFFF]
#define MEM_RAM_INTERNAL_SIZE (0x2000) // internal ram subsection -> [$C000; $DFFF]
#define MEM_RAM_INTERNAL_ECHO_SIZE (0x1E00)
#define MEM_SPRITE_ATTRI_SIZE (0x00A0) // OAM
#define MEM_UNUSABLE_SIZE (0x0060)
#define MEM_IO_PORTS_SIZE (0x0100) // [$FF00; $FFFF] -> contains HRAM, easier for SFR structure
#define MEM_HRAM_SIZE (0x7F)

#define MEM_ROM_BIOS_OFFSET (0x0000)
#define MEM_ROM_BANK_0_OFFSET (0x0000)
#define MEM_ROM_SWITCH_BANK_OFFSET (0x4000)
#define MEM_VIDEO_RAM_OFFSET (0x8000)
#define MEM_RAM_SWITCH_OFFSET (0xA000)
#define MEM_RAM_INTERNAL_OFFSET (0xC000)
#define MEM_RAM_INTERNAL_ECHO_OFFSET (0xE000)
#define MEM_SPRITE_ATTRI_OFFSET (0xFE00)
#define MEM_UNUSABLE_OFFSET (0xFEA0)
#define MEM_IO_PORTS_OFFSET (0xFF00)
#define MEM_HRAM_OFFSET (0xFF80)
#define MEM_IE_REG_OFFSET (0xFFFF)


// Memory map structure
typedef struct{
	Memory mem; // RAM, ROM, VRAM, ...
	uint16_t offset; // address offset of memory map
}MemoryMap;


#endif

