#include "cpu.h"
#include "opcode.h"

Cpu* cpu_Init(void){
	Cpu *pCpu = NULL;
	pCpu = (Cpu*)malloc(sizeof(Cpu));
	if (!pCpu)
		return NULL;
	cpu_Reset(pCpu);
	pCpu->map = NULL;
	pCpu->map = (MemoryMap*)malloc(sizeof(MemoryMap) * MEM_ADDRESS_SPACES);
	pCpu->reg[REG_B] = (union Cpu_Register*)&pCpu->B;
	pCpu->reg[REG_C] = (union Cpu_Register*)&pCpu->C;
	pCpu->reg[REG_D] = (union Cpu_Register*)&pCpu->D;
	pCpu->reg[REG_E] = (union Cpu_Register*)&pCpu->E;
	pCpu->reg[REG_H] = (union Cpu_Register*)&pCpu->H;
	pCpu->reg[REG_L] = (union Cpu_Register*)&pCpu->L;
	pCpu->reg[REG_A] = (union Cpu_Register*)&pCpu->A; // reg[7]
	pCpu->reg[REG_F] = (union Cpu_Register*)&pCpu->F;
	return pCpu;
}

void cpu_Free(Cpu *pCpu){
	free(pCpu->map);
	free(pCpu);
	pCpu = NULL;
	return;
}

void cpu_Reset(Cpu *pCpu){
	pCpu->clock_cycle = 0;
	pCpu->AF = 0;
	pCpu->BC = 0;
	pCpu->DE = 0;
	pCpu->HL = 0;
	pCpu->PC = 0;
	pCpu->SP = 0xFFFE; // Game Boy cpu manual p64
	pCpu->sfr = (union Special_Register*)NULL;
	return;
}

void cpu_SetSpecialRegisters(Cpu *pCpu, uint8_t *pMem){
	pCpu->sfr = (union Special_Register*)pMem;
	return;
}

void cpu_ExecuteOpcode(Cpu *pCpu){
	uint8_t opcode = 0;
	uint8_t extended = 0;

	// TODO: Check for interrupt

	// TODO: Use fetch-decode-execute to read opcode and execute instruction
	pCpu->address_bus = pCpu->PC;
	if (pCpu->sfr->BIOS != 0){
		opcode = pCpu->map[MAP_ROM_BANK_0].mem.data[pCpu->address_bus];
	}else{
		opcode = pCpu->map[MAP_ROM_BIOS].mem.data[pCpu->address_bus];
	}

	// Opcode info
	printf("$%04X> %02X %s\t\t%s\n", pCpu->PC, opcode, page0[opcode].mnemonic, page0[opcode].description);

	if (page0[opcode].type == EXTENDED){ // opcode is 0xCB
		extended = 1;
        // TODO: Use fetch-decode-execute to read opcode and execute instruction
        pCpu->address_bus = pCpu->PC;
        if (pCpu->sfr->BIOS != 0){
            opcode = pCpu->map[MAP_ROM_BANK_0].mem.data[pCpu->address_bus + 1];
        }else{
            opcode = pCpu->map[MAP_ROM_BIOS].mem.data[pCpu->address_bus + 1];
        }
		printf("$%04X> %02X %s\t\t%s\n", pCpu->PC, opcode, page1[opcode].mnemonic, page1[opcode].description);
	}

	// TODO: Execute all opcodes
	// TODO: Fix all (HL) instructions that read data from RAM, currently these are commented out
	if (extended){ // page1 opcodes
		uint8_t bit, r, mask, dummy;
		switch (opcode & 0xF8){
			case 0x00: // RLC 9 bit rotate left with carry
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				dummy = pCpu->FLAG_bits.C;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_7;
					pCpu->reg[r]->R = dummy | ((pCpu->reg[r]->R << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x80;
					// pMem[pCpu->HL] = dummy | ((pMem[pCpu->HL] << 1) & 0xFE);
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
				}
				break;
			case 0x08: // RRC 9 bit rotate right with carry
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				dummy = pCpu->FLAG_bits.C;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_0;
					pCpu->reg[r]->R = (dummy << 7) | ((pCpu->reg[r]->R >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x01;
					// pMem[pCpu->HL] = (dummy << 7) | ((pMem[pCpu->HL] >> 1) & 0x7F);
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
				}
				break;
			case 0x10: // RL 8 bit rotate left
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_7;
					pCpu->reg[r]->R = pCpu->FLAG_bits.C | ((pCpu->reg[r]->R << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x80;
					// pMem[pCpu->HL] = pCpu->FLAG_bits.C | ((pMem[pCpu->HL] << 1) & 0xFE);
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
				}
				break;
			case 0x18: // RR 8 bit rotate right
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_0;
					pCpu->reg[r]->R = (pCpu->FLAG_bits.C << 7) | ((pCpu->reg[r]->R >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x01;
					// pMem[pCpu->HL] = (pCpu->FLAG_bits.C << 7) | ((pMem[pCpu->HL] >> 1) & 0x7F);
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
				}
				break;
			case 0x20: // SLA
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_7;
					pCpu->reg[r]->R = (pCpu->reg[r]->R << 1) & 0xFE;
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x80;
					// pMem[pCpu->HL] = (pMem[pCpu->HL] << 1) & 0xFE;
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
				}
				break;
			case 0x28: // SRA
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_0;
					pCpu->reg[r]->R = (0x80 & pCpu->reg[r]->R) | ((pCpu->reg[r]->R >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x01;
					// pMem[pCpu->HL] = (0x80 & pMem[pCpu->HL]) | ((pMem[pCpu->HL] >> 1) & 0x7F);
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
				}
				break;
			case 0x30: // SWAP
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.C = 0;

#define SWAP(x) (x) = ((((x) & 0xF0) >> 4) | (((x) & 0x0F) << 4))

				r = opcode & 0x07;
				if (r != 0x06){
					SWAP(pCpu->reg[r]->R);
					pCpu->FLAG_bits.Z = !(pCpu->reg[r]->R);
				}else{
					// SWAP(pMem[pCpu->HL]);
					// pCpu->FLAG_bits.Z = !(pMem[pCpu->HL]);
				}
				break;
			case 0x38: // SRL
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r = opcode & 0x07;
				if (r != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r]->R_bits.bit_0;
					pCpu->reg[r]->R = 0x7F & (pCpu->reg[r]->R >> 1);
					pCpu->FLAG_bits.Z = pCpu->reg[r]->R == 0;
				}else{
					// pCpu->FLAG_bits.C = pMem[pCpu->HL] & 0x01;
					// pMem[pCpu->HL] = 0x7F & (pMem[pCpu->HL] >> 1);
					// pCpu->FLAG_bits.Z = pMem[pCpu->HL] == 0;
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
				if (r != 0x06)
					pCpu->FLAG_bits.Z = !(pCpu->reg[r]->R & mask);
				else
					// pCpu->FLAG_bits.Z = !(pMem[pCpu->HL] & mask);
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
				if (r != 0x06)
					pCpu->reg[r]->R &= ~mask;
				else
					// pMem[pCpu->HL] &= ~mask;
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
				if (r != 0x06)
					pCpu->reg[r]->R |= mask;
				else
					// pMem[pCpu->HL] |= mask;
				break;
			default:
				printf("unknown instruction CB%02X\n", opcode);
				break;
		}
	}else{ // page0 opcodes
		switch (opcode){
            default:
                break;
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
