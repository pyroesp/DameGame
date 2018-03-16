#include "cpu.h"
#include "opcode.h"

Cpu* cpu_Init(uint8_t *mem){
	Cpu *pCpu = NULL;
	pCpu = (Cpu*)malloc(sizeof(Cpu));
	if (!pCpu)
		return NULL;
	cpu_Reset(pCpu);
	cpu_Set_Special_Registers(pCpu, mem);
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
		// Check endianess
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
				break;
			case 0x30: // SWAP
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.C = 0;

#define SWAP(x) x = (((x & 0xF0) >> 4) | ((x & 0x0F) << 4))

				r = opcode & 0x07;
				switch (r){
					case 0x0: // reg B
						SWAP(pCpu->B);
						pCpu->FLAG_bits.Z = !(pCpu->B);
						break;
					case 0x1: // reg C
						SWAP(pCpu->C);
						pCpu->FLAG_bits.Z = !(pCpu->C);
						break;
					case 0x2: // reg D
						SWAP(pCpu->D);
						pCpu->FLAG_bits.Z = !(pCpu->D);
						break;
					case 0x3: // reg E
						SWAP(pCpu->E);
						pCpu->FLAG_bits.Z = !(pCpu->E);
						break;
					case 0x4: // reg H
						SWAP(pCpu->H);
						pCpu->FLAG_bits.Z = !(pCpu->H);
						break;
					case 0x5: // reg L
						SWAP(pCpu->L);
						pCpu->FLAG_bits.Z = !(pCpu->L);
						break;
					case 0x6: // reg (HL)
						SWAP(mem[pCpu->HL]);
						pCpu->FLAG_bits.Z = !(mem[pCpu->HL]);
						break;
					case 0x7: // reg A
						SWAP(pCpu->A);
						pCpu->FLAG_bits.Z = !(pCpu->A);
						break;
				}
				break;
			case 0x38: // SRL
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				switch (r){
					case 0x0: // reg B
						pCpu->FLAG_bits.C = pCpu->B & 0x01;
						pCpu->B = 0x7F & (pCpu->B >> 1);
						pCpu->FLAG_bits.Z = pCpu->B == 0;
						break;
					case 0x1: // reg C
						pCpu->FLAG_bits.C = pCpu->C & 0x01;
						pCpu->C = 0x7F & (pCpu->C >> 1);
						pCpu->FLAG_bits.Z = pCpu->C == 0;
						break;
					case 0x2: // reg D
						pCpu->FLAG_bits.C = pCpu->D & 0x01;
						pCpu->D = 0x7F & (pCpu->D >> 1);
						pCpu->FLAG_bits.Z = pCpu->D == 0;
						break;
					case 0x3: // reg E
						pCpu->FLAG_bits.C = pCpu->E & 0x01;
						pCpu->E = 0x7F & (pCpu->E >> 1);
						pCpu->FLAG_bits.Z = pCpu->E == 0;
						break;
					case 0x4: // reg H
						pCpu->FLAG_bits.C = pCpu->H & 0x01;
						pCpu->H = 0x7F & (pCpu->H >> 1);
						pCpu->FLAG_bits.Z = pCpu->H == 0;
						break;
					case 0x5: // reg L
						pCpu->FLAG_bits.C = pCpu->L & 0x01;
						pCpu->L = 0x7F & (pCpu->L >> 1);
						pCpu->FLAG_bits.Z = pCpu->L == 0;
						break;
					case 0x6: // reg (HL)
						pCpu->FLAG_bits.C = mem[pCpu->HL] & 0x01;
						mem[pCpu->HL] = 0x7F & (mem[pCpu->HL] >> 1);
						pCpu->FLAG_bits.Z = mem[pCpu->HL] == 0;
						break;
					case 0x7: // reg A
						pCpu->FLAG_bits.C = pCpu->A & 0x01;
						pCpu->A = 0x7F & (pCpu->A >> 1);
						pCpu->FLAG_bits.Z = pCpu->A == 0;
						break;
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
				switch (r){
					case 0x0: // reg B
						pCpu->FLAG_bits.Z = !(pCpu->B & mask);
						break;
					case 0x1: // reg C
						pCpu->FLAG_bits.Z = !(pCpu->C & mask);
						break;
					case 0x2: // reg D
						pCpu->FLAG_bits.Z = !(pCpu->D & mask);
						break;
					case 0x3: // reg E
						pCpu->FLAG_bits.Z = !(pCpu->E & mask);
						break;
					case 0x4: // reg H
						pCpu->FLAG_bits.Z = !(pCpu->H & mask);
						break;
					case 0x5: // reg L
						pCpu->FLAG_bits.Z = !(pCpu->L & mask);
						break;
					case 0x6: // reg (HL)
						pCpu->FLAG_bits.Z = !(mem[pCpu->HL] & mask);
						break;
					case 0x7: // reg A
						pCpu->FLAG_bits.Z = !(pCpu->A & mask);
						break;
				}
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
				switch (r){
					case 0x0: // reg B
						pCpu->B &= ~mask;
						break;
					case 0x1: // reg C
						pCpu->C &= ~mask;
						break;
					case 0x2: // reg D
						pCpu->D &= ~mask;
						break;
					case 0x3: // reg E
						pCpu->E &= ~mask;
						break;
					case 0x4: // reg H
						pCpu->H &= ~mask;
						break;
					case 0x5: // reg L
						pCpu->L &= ~mask;
						break;
					case 0x6: // reg (HL)
						mem[pCpu->HL] &= ~mask;
						break;
					case 0x7: // reg A
						pCpu->A &= ~mask;
						break;
				}
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

				/*
				TODO : Check if Cpu->reg is correctly allign so that
						Cpu->reg[0] is the same as Cpu->B
						Cpu->reg[1] is the same as Cpu->C
						...
						If so, then change all switch (r){} to if statement below

						if (r < 0x06)
							pCpu->reg[r] |= mask;
						else if (r == 0x06)
							mem[pCpu->HL] |= mask;
						else if (r == 0x07)
							pCpu->A |= mask;
				*/

				switch (r){
					case 0x0: // reg B
						pCpu->B |= mask;
						break;
					case 0x1: // reg C
						pCpu->C |= mask;
						break;
					case 0x2: // reg D
						pCpu->D |= mask;
						break;
					case 0x3: // reg E
						pCpu->E |= mask;
						break;
					case 0x4: // reg H
						pCpu->H |= mask;
						break;
					case 0x5: // reg L
						pCpu->L |= mask;
						break;
					case 0x6: // reg (HL)
						mem[pCpu->HL] |= mask;
						break;
					case 0x7: // reg A
						pCpu->A |= mask;
						break;
				}
				break;
			default:
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
