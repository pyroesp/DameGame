#include "cpu.h"
#include "opcode.h"

Cpu* cpu_Init(uint8_t *mem){
	Cpu *pCpu = NULL;
	pCpu = (Cpu*)malloc(sizeof(Cpu));
	if (!pCpu)
		return NULL;
	cpu_Reset(pCpu);
	cpu_Set_Special_Registers(pCpu, mem);
	pCpu->reg[0] = &pCpu->B;
	pCpu->reg[1] = &pCpu->C;
	pCpu->reg[2] = &pCpu->D;
	pCpu->reg[3] = &pCpu->E;
	pCpu->reg[4] = &pCpu->H;
	pCpu->reg[5] = &pCpu->L;
	pCpu->reg[6] = &pCpu->A;
	pCpu->reg[7] = &pCpu->F;
	return pCpu;
}

void cpu_Free(Cpu *pCpu){
	free(pCpu);
	pCpu = NULL;
}

void cpu_Reset(Cpu *pCpu){
	pCpu->clock_cycle = 0;
	pCpu->AF = 0;
	pCpu->BC = 0;
	pCpu->DE = 0;
	pCpu->HL = 0;
	pCpu->PC = 0;
	pCpu->SP = 0;
	pCpu->sfr = (union Special_Register*)NULL;
}

void cpu_Set_Special_Registers(Cpu *pCpu, uint8_t *mem){
	pCpu->sfr = (union Special_Register*)&mem[MEM_IO_PORTS_OFFSET];
	return;
}

void cpu_Execute_Opcode(Cpu *pCpu, uint8_t *mem){
	uint8_t opcode = 0;
	uint16_t arg_word;
	uint8_t arg_byte;
	uint8_t extended = 0;

	// TODO: Check for interrupt

	// TODO: Get opcode and arg
	opcode = mem[pCpu->PC];
	if (page0[opcode].size == 2)
		arg_byte = mem[pCpu->PC + 1];
	else if (page0[opcode].size == 3)
		// Check endianness
		arg_word = mem[pCpu->PC + 1] << 8 | mem[pCpu->PC + 2];

	printf("$%04X> %02X %s\t\t%s\n", pCpu->PC, opcode, page0[opcode].mnemonic, page0[opcode].description);

	if (page0[opcode].type == EXTENDED){ // opcode is 0xCB
		extended = 1;
		opcode = mem[pCpu->PC + 1];
		printf("$%04X> %02X %s\t\t%s\n", pCpu->PC, opcode, page1[opcode].mnemonic, page1[opcode].description);
	}

	// TODO: Execute opcode
	if (extended){ // page1 opcodes
		uint8_t bit, r, mask;
		switch (opcode & 0xF8){
			case 0x00: // RLC
				break;
			case 0x08: // RRC
				break;
			case 0x10: // RL
				break;
			case 0x18: // RR
				break;
			case 0x20: // SLA
				break;
			case 0x28: // SRA
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r < 0x06){
					pCpu->FLAG_bits.C = *(pCpu->reg[r]) & 0x01;
					*(pCpu->reg[r]) = (0x80 & *(pCpu->reg[r])) | (*(pCpu->reg[r]) >> 1);
					pCpu->FLAG_bits.Z = *(pCpu->reg[r]) == 0;
				} else if (r == 0x06){
					pCpu->FLAG_bits.C = mem[pCpu->HL] & 0x01;
					mem[pCpu->HL] = (0x80 & mem[pCpu->HL]) | (mem[pCpu->HL] >> 1);
					pCpu->FLAG_bits.Z = mem[pCpu->HL] == 0;
				} else if (r == 0x07){
					pCpu->FLAG_bits.C = pCpu->A & 0x01;
					pCpu->A = (0x80 & pCpu->A) | (pCpu->A >> 1);
					pCpu->FLAG_bits.Z = pCpu->A == 0;
				}
				break;
			case 0x30: // SWAP
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.C = 0;

#define SWAP(x) (x) = ((((x) & 0xF0) >> 4) | (((x) & 0x0F) << 4))

				r = opcode & 0x07;
				if (r < 0x06){
					SWAP(*(pCpu->reg[r]));
					pCpu->FLAG_bits.Z = !(*(pCpu->reg[r]));
				} else if (r == 0x06){
					SWAP(mem[pCpu->HL]);
					pCpu->FLAG_bits.Z = !(mem[pCpu->HL]);
				} else if (r == 0x07){
					SWAP(pCpu->A);
					pCpu->FLAG_bits.Z = !(pCpu->A);
				}
				break;
			case 0x38: // SRL
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r < 0x06){
					pCpu->FLAG_bits.C = *(pCpu->reg[r]) & 0x01;
					*(pCpu->reg[r]) = 0x7F & (*(pCpu->reg[r]) >> 1);
					pCpu->FLAG_bits.Z = *(pCpu->reg[r]) == 0;
				} else if (r == 0x06){
					pCpu->FLAG_bits.C = mem[pCpu->HL] & 0x01;
					mem[pCpu->HL] = 0x7F & (mem[pCpu->HL] >> 1);
					pCpu->FLAG_bits.Z = mem[pCpu->HL] == 0;
				} else if (r == 0x07){
					pCpu->FLAG_bits.C = pCpu->A & 0x01;
					pCpu->A = 0x7F & (pCpu->A >> 1);
					pCpu->FLAG_bits.Z = pCpu->A == 0;
				}
				break;
			case 0x40: // BIT 0
			case 0x48: // BIT 1
			case 0x50: // BIT 2
			case 0x58: // BIT 3
			case 0x60: // BIT 4
			case 0x68: // BIT 5
			case 0x70: // BIT 6
			case 0x78: // BIT 7
				bit = ((opcode & 0xF0) - 0x40) * 2;
				bit = bit + (opcode & 0x08) ? 1 : 0;
				mask = 1 << bit;

				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 1;

				r = opcode & 0x07;
				if (r < 0x06)
					pCpu->FLAG_bits.Z = !(*(pCpu->reg[r]) & mask);
				else if (r == 0x06)
					pCpu->FLAG_bits.Z = !(mem[pCpu->HL] & mask);
				else if (r == 0x07)
					pCpu->FLAG_bits.Z = !(pCpu->A & mask);
				break;
			case 0x80: // RES 0
			case 0x88: // RES 1
			case 0x90: // RES 2
			case 0x98: // RES 3
			case 0xA0: // RES 4
			case 0xA8: // RES 5
			case 0xB0: // RES 6
			case 0xB8: // RES 7
				bit = ((opcode & 0xF0) - 0x80) * 2;
				bit = bit + (opcode & 0x08) ? 1 : 0;
				mask = 1 << bit;

				r = opcode & 0x07;
				if (r < 0x06)
					*(pCpu->reg[r]) &= ~mask;
				else if (r == 0x06)
					mem[pCpu->HL] &= ~mask;
				else if (r == 0x07)
					pCpu->A &= ~mask;
				break;
			case 0xC0: // SET 0
			case 0xC8: // SET 1
			case 0xD0: // SET 2
			case 0xD8: // SET 3
			case 0xE0: // SET 4
			case 0xE8: // SET 5
			case 0xF0: // SET 6
			case 0xF8: // SET 7
				bit = ((opcode & 0xF0) - 0xC0) * 2;
				bit = bit + (opcode & 0x08) ? 1 : 0;
				mask = 1 << bit;

				r = opcode & 0x07;
				if (r < 0x06)
					*(pCpu->reg[r]) |= mask;
				else if (r == 0x06)
					mem[pCpu->HL] |= mask;
				else if (r == 0x07)
					pCpu->A |= mask;
				break;
			default:
				printf("unknown instruction CB%02X\n", opcode);
				break;
		}
	}else{ // page0 opcodes
		switch (opcode){

		}
	}


	// Increase clock_cycle and PC
	if (extended){
		pCpu->clock_cycle += page1[pCpu->PC + 1].clock_cycles;
		pCpu->PC += page1[pCpu->PC + 1].size;
	}else{
		pCpu->clock_cycle += page0[pCpu->PC].clock_cycles;
		pCpu->PC += page0[pCpu->PC].size;
	}
}
