#ifndef _MEMORY_MAP_H
#define _MEMORY_MAP_H

#include "rom.h"
#include "ram.h"

/*
	Interrupt Enable Register
	--------------------------- FFFF
	Internal RAM
	--------------------------- FF80
	Empty but unusable for I/O
	--------------------------- FF4C
	I/O ports
	--------------------------- FF00
	Empty but unusable for I/O
	--------------------------- FEA0
	Sprite Attrib Memory (OAM)
	--------------------------- FE00
	Echo of 8kB Internal RAM
	--------------------------- E000
	8kB Internal RAM
	--------------------------- C000
	8kB switchable RAM bank
	--------------------------- A000
	8kB Video RAM
	--------------------------- 8000
	16kB switchable ROM bank
	--------------------------- 4000
	16kB ROM bank #0
	--------------------------- 0000
*/

#define MEM_TOTAL_SIZE (0x10000)

#define MEM_RAM_INTERNAL_ECHO_SIZE (0x1E00)
#define MEM_SPRITE_ATTRI_SIZE (0x00A0)
#define MEM_UNUSABLE_SIZE (0x0060)
#define MEM_IO_PORTS_SIZE (0x00FF)

#define MEM_ROM_BANK0_OFFSET (0x0000)
#define MEM_ROM_SWITCH_BANK_OFFSET (0x4000)
#define MEM_RAM_VIDEO_OFFSET (0x8000)
#define MEM_RAM_SWITCH_OFFSET (0xA000)
#define MEM_RAM_INTERNAL_OFFSET (0xC000)
#define MEM_RAM_INTERNAL_ECHO_OFFSET (0xE000)
#define MEM_SPRITE_ATTRI_OFFSET (0xFE00)
#define MEM_UNUSABLE_OFFSET (0xFEA0)
#define MEM_IO_PORTS_OFFSET (0xFF00)
#define MEM_RAM_INTERNAL2_OFFSET (0xFF80)

#endif

