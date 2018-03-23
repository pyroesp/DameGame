#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "special_register.h"
#include "interrupt.h"
#include "memory_map.h"

/*

	8 bit LR35902 CPU - Z80 & 8080 hybrid @4.194304MHz

*/

#define REG_B (0)
#define REG_C (1)
#define REG_D (2)
#define REG_E (3)
#define REG_H (4)
#define REG_L (5)
#define REG_A (7)
#define REG_F (6)

union Cpu_Register{
	uint8_t R;
	struct{
		uint8_t bit_0 : 1;
		uint8_t bit_1 : 1;
		uint8_t bit_2 : 1;
		uint8_t bit_3 : 1;

		uint8_t bit_4 : 1;
		uint8_t bit_5 : 1;
		uint8_t bit_6 : 1;
		uint8_t bit_7 : 1;
	}R_bits;
};

typedef struct{
	uint64_t clock_cycle; // 4 x machine cycle

	union{
		uint16_t BC;
		struct{
			uint8_t C;
			uint8_t B;
		};
	};
	union{
		uint16_t DE;
		struct{
			uint8_t E;
			uint8_t D;
		};
	};
	union{
		uint16_t HL;
		struct{
			uint8_t L;
			uint8_t H;
		};
	};
	union{
		uint16_t AF;
		struct{
			union{
				uint8_t F;
				union{
                    uint8_t FLAG;
                    struct{
                        uint8_t unused : 4;
                        uint8_t C : 1;
                        uint8_t H : 1;
                        uint8_t N : 1;
                        uint8_t Z : 1;
                    }FLAG_bits;
				};
				union{
                    uint8_t STATUS;
                    struct{
                        uint8_t unused : 4;
                        uint8_t carry : 1;
                        uint8_t half_carry : 1;
                        uint8_t substract : 1;
                        uint8_t zero : 1;
                    }STATUS_flags;
				};
			};
			uint8_t A;
		};
	};
	union Cpu_Register *reg[8];

	uint16_t SP; // decrements before putting something on the stack
	uint16_t PC;

	uint16_t address_bus;
	uint8_t data_bus;

	union Special_Register *sfr;
}Cpu;


Cpu* cpu_Init(uint8_t *pMem);
void cpu_Free(Cpu *pCpu);
void cpu_Reset(Cpu *pCpu);
void cpu_Set_Special_Registers(Cpu *pCpu, uint8_t *pMem);
void cpu_Execute_Opcode(Cpu *pCpu, uint8_t *pMem);

#endif

/*
Power Up Sequence
-----------------

  When the GameBoy is powered up, a 256 byte program
 starting at memory location 0 is executed. This program
 is located in a ROM inside the GameBoy. The first thing
 the program does is read the cartridge locations from
 $104 to $133 and place this graphic of a Nintendo logo
 on the screen at the top. This image is then scrolled
 until it is in the middle of the screen. Two musical
 notes are then played on the internal speaker. Again,
 the cartridge locations $104 to $133 are read but this
 time they are compared with a table in the internal rom.
 If any byte fails to compare, then the GameBoy stops
 comparing bytes and simply halts all operations. If all
 locations compare the same, then the GameBoy starts
 adding all of the bytes in the cartridge from $134 to
 $14d. A value of 25 decimal is added to this total. If
 the least significant byte of the result is a not a
 zero, then the GameBoy will stop doing anything. If it
 is a zero, then the internal ROM is disabled and
 cartridge program execution begins at location $100
 with AF=$01B0, BC=$0013, DE=$00D8, HL=$014D and
 Stack Pointer=$FFFE.

 */
