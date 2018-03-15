#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "special_register.h"
#include "memory_map.h"
#include "interrupt.h"

/*

	8 bit - Z80 like CPU @4.194304MHz

*/

typedef struct{
	uint64_t clock_cycle; // 4 x machine cycle

	union{
		// TODO: Check if correctly alligned so that
		// 0 to 5 -> B, C, D, E, H, L
		uint8_t reg[8]; // Easier for executing opcodes
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
					uint8_t FLAG;
					struct{
						uint8_t unused : 4;
						uint8_t C : 1;
						uint8_t H : 1;
						uint8_t N : 1;
						uint8_t Z : 1;
					}FLAG_bits;
					uint8_t STATUS;
					struct{
						uint8_t unused : 4;
						uint8_t carry : 1;
						uint8_t half_carry : 1;
						uint8_t substract : 1;
						uint8_t zero : 1;
					}STATUS_flags;
				};
				uint8_t A;
			};
		};
	};
	uint16_t SP; // decrements before putting something on the stack
	uint16_t PC;
	union Special_Register *sfr;
}Cpu;


Cpu* cpu_Init(uint8_t *mem);
void cpu_Free(Cpu *pCpu);
void cpu_Reset(Cpu *pCpu);
void cpu_Set_Special_Registers(Cpu *pCpu, uint8_t *mem);
void cpu_Execute_Opcode(Cpu *pCpu, uint8_t *mem);

#endif
