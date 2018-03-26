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
	pCpu->extended = 0;
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

uint8_t* cpu_GetByte(Cpu *pCpu){ // read byte at address_bus into data_bus, return pointer to byte in memory
	uint8_t *byte = NULL;
	MemoryMap *map = NULL;

	// TODO : Bin search correct memory space ?
	if (pCpu->address_bus < MEM_ROM_SWITCH_BANK_OFFSET){ // BIOS & ROM bank 0
		if (pCpu->sfr->BIOS){ // BIOS disabled
			map = &pCpu->map[MAP_ROM_BANK_0];
		}else{
			map = &pCpu->map[MEM_ROM_BIOS_SIZE];
		}
	}else if (pCpu->address_bus < MEM_VIDEO_RAM_OFFSET){ // ROM bank switch
		map = &pCpu->map[MAP_ROM_BANK_SWITCH];
	}else if (pCpu->address_bus < MEM_RAM_SWITCH_OFFSET){ // VRAM
		map = &pCpu->map[MAP_VRAM];
	}else if (pCpu->address_bus < MEM_RAM_INTERNAL_OFFSET){ // RAM bank switch
		map = &pCpu->map[MAP_RAM_BANK_SWITCH];
	}else if (pCpu->address_bus < MEM_RAM_INTERNAL_ECHO_OFFSET){ // Internal RAM
		map = &pCpu->map[MAP_RAM_INTERNAL];
	}else if (pCpu->address_bus < MEM_SPRITE_ATTRI_OFFSET){ // Internal RAM echo
		map = &pCpu->map[MAP_RAM_INTERNAL_ECHO];
	}else if (pCpu->address_bus < MEM_UNUSABLE_OFFSET){ // Object attribute ram
		map = &pCpu->map[MAP_OAM];
	}else{
		if (pCpu->address_bus < MEM_IO_PORTS_OFFSET){ // Unusable memory
			printf("Illegal address> $%04X\n", pCpu->address_bus);
			return NULL;
		}
		if (pCpu->address_bus >= MEM_HRAM_OFFSET && pCpu->address_bus < MEM_HRAM_OFFSET + MEM_HRAM_SIZE){ // HRAM
			map = &pCpu->map[MAP_HRAM];
		}else if (pCpu->address_bus <= MEM_IE_REG_OFFSET){
			map = &pCpu->map[MAP_IO_PORTS];
		}else{
			printf("Illegal address> $%04X\n", pCpu->address_bus);
			return NULL;
		}
	}

	if (map != NULL){
		byte = &map->mem.data[(pCpu->address_bus - map->offset) + map->mem.start_idx];
		pCpu->data_bus = (*byte);
		return byte;
	}else{
		printf("map pointer is null for some reason\n");
		return NULL;
	}
}

void cpu_Run(Cpu *pCpu){
	uint8_t opcode;
	uint8_t *byte = NULL;

	// TODO: Check for interrupt

	// TODO: Use fetch-decode-execute to read opcode and execute instruction
	// Read opcode first
	pCpu->address_bus = pCpu->PC;
	byte = cpu_GetByte(pCpu);
	if (byte == NULL){
		printf("GetByte returned error\n");
		return;
	}else{
        if (pCpu->data_bus == OPCODE_EXTENDED){
            pCpu->extended = 1;
			pCpu->address_bus = pCpu->PC + 1;
			byte = cpu_GetByte(pCpu);
			if (byte == NULL){
				printf("GetByte returned error\n");
				return;
			}
			printf("$%04X> %02X %s\t\t%s\n", pCpu->address_bus, pCpu->data_bus, page1[pCpu->data_bus].mnemonic, page1[pCpu->data_bus].description);
		}else{
            printf("$%04X> %02X %s\t\t%s\n", pCpu->address_bus, pCpu->data_bus, page0[pCpu->data_bus].mnemonic, page0[pCpu->data_bus].description);
        }
		opcode = pCpu->data_bus;
	}

	// TODO: Execute all opcodes
	// TODO: Fix all (HL) instructions that read data from RAM, currently these are commented out
	if (pCpu->extended){ // page1 opcodes
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x80;
					*byte = dummy | ((pCpu->data_bus << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					*byte = (dummy << 7) | ((pCpu->data_bus >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x80;
					*byte = pCpu->FLAG_bits.C | ((pCpu->data_bus << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					*byte = (pCpu->FLAG_bits.C << 7) | ((pCpu->data_bus >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x80;
					*byte = (pCpu->data_bus << 1) & 0xFE;
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					*byte = (0x80 & pCpu->data_bus) | ((pCpu->data_bus >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					SWAP(pCpu->data_bus);
					pCpu->FLAG_bits.Z = !(pCpu->data_bus);
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
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					*byte = 0x7F & (pCpu->data_bus >> 1);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
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
				else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.Z = !(pCpu->data_bus & mask);
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
				if (r != 0x06)
					pCpu->reg[r]->R &= ~mask;
				else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					*byte = pCpu->data_bus & ~mask;
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
				if (r != 0x06)
					pCpu->reg[r]->R |= mask;
				else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						printf("cpu_GetByte failed\n");
						return;
					}
					*byte = pCpu->data_bus | mask;
				}
				break;
			default:
				printf("unknown instruction #%X\n", opcode & 0xF8);
				break;
		}
		// reset extended mode
		pCpu->extended = 0;
		// increase clock cycle and PC
		pCpu->clock_cycle += page1[pCpu->PC + 1].clock_cycles;
		pCpu->PC += page1[pCpu->PC + 1].size;
	}else{ // page0 opcodes
		switch (opcode){
            default:
                break;
		}
		// increase clock cycle and PC
		pCpu->clock_cycle += page0[pCpu->PC].clock_cycles;
		pCpu->PC += page0[pCpu->PC].size;
	}
}
