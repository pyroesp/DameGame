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
	union Special_Register *sfr;
}Cpu;


Cpu* cpu_Init(uint8_t *mem);
void cpu_Free(Cpu *pCpu);
void cpu_Reset(Cpu *pCpu);
void cpu_Set_Special_Registers(Cpu *pCpu, uint8_t *mem);
void cpu_Execute_Opcode(Cpu *pCpu, uint8_t *mem);

#endif
