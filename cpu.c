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
	pCpu->dreg[REG_BC] = &pCpu->BC;
	pCpu->dreg[REG_DE] = &pCpu->DE;
	pCpu->dreg[REG_HL] = &pCpu->HL;
	pCpu->dreg[REG_SP] = &pCpu->SP;
	pCpu->dreg[REG_AF] = &pCpu->AF;
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
	pCpu->stop = 0;
	pCpu->halt = 0;
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

void cpu_SetInterruptEnableRegister(Cpu *pCpu, uint8_t *pMem){
	pCpu->ie_reg = (union Interrupt_Enable*)pMem;
	return;
}

uint8_t* cpu_GetByte(Cpu *pCpu){ // read byte at address_bus into data_bus, return pointer to byte in memory
	uint8_t (*byte) = NULL;
	MemoryMap *map = NULL;

	// TODO : Bin search correct memory space ?
	if (pCpu->address_bus < MEM_ROM_SWITCH_BANK_OFFSET){ // BIOS & ROM bank 0
		if (pCpu->sfr->BIOS){ // BIOS disabled
			map = &pCpu->map[MAP_ROM_BANK_0];
		}else{
			map = &pCpu->map[MAP_ROM_BIOS];
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
			DEBUG_PRINTF("Illegal address> $%04X\n", pCpu->address_bus);
			return NULL;
		}
		if (pCpu->address_bus >= MEM_HRAM_OFFSET && pCpu->address_bus < MEM_HRAM_OFFSET + MEM_HRAM_SIZE){ // HRAM
			map = &pCpu->map[MAP_HRAM];
		}else if (pCpu->address_bus <= MEM_IE_REG_OFFSET){
			map = &pCpu->map[MAP_IO_PORTS];
		}else{
			DEBUG_PRINTF("Illegal address> $%04X\n", pCpu->address_bus);
			return NULL;
		}
	}

	if (map != NULL){
		byte = &map->mem.data[(pCpu->address_bus - map->offset) + map->mem.start_idx];
		pCpu->data_bus = (*byte);
		return byte;
	}else{
		DEBUG_PRINTF("map pointer is null for some reason\n");
		return NULL;
	}
}

uint16_t cpu_GetWordFromPC(Cpu *pCpu){
	uint16_t word = 0;
	cpu_GetByte(pCpu);
	word = pCpu->data_bus;
	pCpu->address_bus = pCpu->PC + 2;
	cpu_GetByte(pCpu);
	word = ((pCpu->data_bus << 8) & 0xFF00) | word;
	return word;
}

uint16_t cpu_Pop(Cpu *pCpu){
	uint16_t pop;
	pCpu->address_bus = pCpu->SP;
	cpu_GetByte(pCpu);
	pop = pCpu->data_bus;
	pCpu->address_bus = pCpu->SP + 1;
	cpu_GetByte(pCpu);
	pop = pop | pCpu->data_bus << 8;
	pCpu->SP += 2;
	return pop;
}

void cpu_Push(Cpu *pCpu, uint16_t var){
    uint8_t (*byte);
	pCpu->address_bus = pCpu->SP - 1;
	byte = cpu_GetByte(pCpu);
	(*byte) = var >> 8 & 0xFF;
	pCpu->address_bus = pCpu->SP - 2;
	byte = cpu_GetByte(pCpu);
	(*byte) = var & 0xFF;
	pCpu->SP -= 2;
}

void cpu_Run(Cpu *pCpu){
	uint8_t opcode;
	uint8_t (*byte) = NULL;
	uint8_t bit, r1, r2, mask, dummy;
	uint16_t word;
	uint8_t jump = 0;

	// TODO: Check for interrupt

	// TODO: Use fetch-decode-execute to read opcode and execute instruction
	// Read opcode first
	pCpu->address_bus = pCpu->PC;
	byte = cpu_GetByte(pCpu);
	if (byte == NULL){
		DEBUG_PRINTF("GetByte returned error\n");
		return;
	}else{
        if (pCpu->data_bus == OPCODE_EXTENDED){
            pCpu->extended = 1;
			pCpu->address_bus = pCpu->PC + 1;
			byte = cpu_GetByte(pCpu);
			if (byte == NULL){
				DEBUG_PRINTF("GetByte returned error\n");
				return;
			}
		}
		opcode = pCpu->data_bus;
	}

	// TODO: Execute all opcodes
	if (!pCpu->extended){ // page0 opcodes
		DEBUG_PRINTF("$%04X> %02X\t", pCpu->PC, opcode);
		switch (opcode){
			case 0x00:// NOP
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Increase instructions */
			case 0x03: // INC BC
			case 0x13: // INC DE
			case 0x23: // INC HL
			case 0x33: // INC SP
				r1 = ((opcode & 0xF0) >> 4);
				(*pCpu->dreg[r1])++;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0x04: // INC B
			case 0x0C: // INC C
			case 0x14: // INC D
			case 0x1C: // INC E
			case 0x24: // INC H
			case 0x2C: // INC L
			case 0x3C: // INC A
				pCpu->FLAG_bits.N = 0;
				r1 = ((opcode >> 4) & 0x0F) * 2 + ((opcode & 0x0F) == 0x0C ? 1 : 0);
				dummy = (pCpu->reg[r1]->R & 0x0F) + 1;
				pCpu->reg[r1]->R++;
				pCpu->FLAG_bits.H = dummy > 0xF;
				pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x34: // INC (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				dummy = (pCpu->data_bus & 0x0F) + 1;
				(*byte) = pCpu->data_bus++;
				pCpu->FLAG_bits.H = dummy > 0xF;
				pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Decrement instructions */
			case 0x05: // DEC B
			case 0x0D: // DEC C
			case 0x15: // DEC D
			case 0x1D: // DEC E
			case 0x25: // DEC H
			case 0x2D: // DEC L
			case 0x3D: // DEC A
				pCpu->FLAG_bits.N = 1;
				r1 = ((opcode >> 4) & 0x0F) * 2 + ((opcode & 0x0F) == 0x0D ? 1 : 0);
				dummy = pCpu->reg[r1]->R & 0x10;
				pCpu->reg[r1]->R--;
				pCpu->FLAG_bits.H = dummy == 0x10;
				pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x35: // DEC (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				dummy = pCpu->data_bus & 0x10;
				(*byte) = pCpu->data_bus--;
				pCpu->FLAG_bits.H = dummy == 0x10;
				pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0x0B: // DEC BC
			case 0x1B: // DEC DE
			case 0x2B: // DEC HL
			case 0x3B: // DEC SP
				r1 = ((opcode & 0xF0) >> 4);
				(*pCpu->dreg[r1])--;
				DEBUG_PRINTF("%s\t\t%s\t", page0[pCpu->data_bus].mnemonic, page0[pCpu->data_bus].description);
				break;

			/* Add instructions */
			case 0x80: // ADD A, B
			case 0x81: // ADD A, C
			case 0x82: // ADD A, D
			case 0x83: // ADD A, E
			case 0x84: // ADD A, H
			case 0x85: // ADD A, L
			case 0x87: // ADD A, A
				r1 = opcode & 0x0F;
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = (pCpu->reg[r1]->R + pCpu->A) > 0xFF;
				pCpu->FLAG_bits.H = ((pCpu->reg[r1]->R & 0x0F) + (pCpu->A & 0x0F)) > 0x0F;
				pCpu->A += pCpu->reg[r1]->R;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x86: // ADD A, (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = (pCpu->data_bus + pCpu->A) > 0xFF;
				pCpu->FLAG_bits.H = ((pCpu->data_bus & 0x0F) + (pCpu->A & 0x0F)) > 0x0F;
				pCpu->A += pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xC6: // ADD A, n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = (pCpu->data_bus + pCpu->A) > 0xFF;
				pCpu->FLAG_bits.H = ((pCpu->data_bus & 0x0F) + (pCpu->A & 0x0F)) > 0x0F;
				pCpu->A += pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			case 0xE8: // ADD SP, n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = (pCpu->data_bus + pCpu->A) > 0xFF;
				pCpu->FLAG_bits.H = ((pCpu->data_bus & 0x0F) + (pCpu->A & 0x0F)) > 0x0F;
				pCpu->SP += pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			case 0x09: // ADD HL, BC
			case 0x19: // ADD HL, DE
			case 0x29: // ADD HL, HL
			case 0x39: // ADD HL, SP
				r1 = ((opcode & 0xF0) >> 4);
				pCpu->FLAG_bits.N = 0;
				word = (pCpu->HL & 0xFFF) + ((*pCpu->dreg[r1]) & 0xFFF);
				pCpu->FLAG_bits.H = word > 0xFFF;
				pCpu->FLAG_bits.C = (pCpu->HL + (*pCpu->dreg[r1])) > 0xFFFF;
				pCpu->HL += (*pCpu->dreg[r1]);
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Add with carry instructions */
			case 0x88: // ADC A, B
			case 0x89: // ADC A, C
			case 0x8A: // ADC A, D
			case 0x8B: // ADC A, E
			case 0x8C: // ADC A, H
			case 0x8D: // ADC A, L
			case 0x8F: // ADC A, A
				r1 = opcode & 0x0F;
				pCpu->FLAG_bits.N = 0;
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = (pCpu->reg[r1]->R + pCpu->A + dummy) > 0xFF;
				pCpu->FLAG_bits.H = ((pCpu->reg[r1]->R & 0x0F) + ((pCpu->A & 0x0F) + dummy)) > 0x0F;
				pCpu->A += pCpu->reg[r1]->R + dummy;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x8E: // ADC A, (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = (pCpu->data_bus + pCpu->A + dummy) > 0xFF;
				pCpu->FLAG_bits.H = ((pCpu->data_bus & 0x0F) + ((pCpu->A & 0x0F) + dummy)) > 0x0F;
				pCpu->A += pCpu->data_bus + dummy;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Sub instructions */
			case 0x90: // SUB B
			case 0x91: // SUB C
			case 0x92: // SUB D
			case 0x93: // SUB E
			case 0x94: // SUB H
			case 0x95: // SUB L
			case 0x97: // SUB A
				r1 = opcode & 0x0F;
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->reg[r1]->R & 0xF0);
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->reg[r1]->R & 0x0F);
				pCpu->A -= pCpu->reg[r1]->R;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x96: // SUB (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->data_bus & 0xF0);
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->data_bus & 0x0F);
				pCpu->A -= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xD6: // SUB n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->data_bus & 0xF0);
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->data_bus & 0x0F);
				pCpu->A -= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			/* Sub with carry instructions */
			case 0x98: // SBC B
			case 0x99: // SBC C
			case 0x9A: // SBC D
			case 0x9B: // SBC E
			case 0x9C: // SBC H
			case 0x9D: // SBC L
			case 0x9F: // SBC A
				r1 = opcode & 0x0F;
				pCpu->FLAG_bits.N = 1;
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->reg[r1]->R & 0xF0);
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->reg[r1]->R & 0x0F);
				pCpu->A -= (pCpu->reg[r1]->R + dummy);
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x9E: // SBC (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->data_bus & 0xF0);
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->data_bus & 0x0F);
				pCpu->A -= (pCpu->data_bus + dummy);
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xDE: // SBC A, n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->data_bus & 0xF0);
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->data_bus & 0x0F);
				pCpu->A -= (pCpu->data_bus + dummy);
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			/* Load instructions */
			case 0x06: // LD B, n
			case 0x0E: // LD C, n
			case 0x16: // LD D, n
			case 0x1E: // LD E, n
			case 0x26: // LD H, n
			case 0x2E: // LD L, n
			case 0x3E: // LD A, n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				r1 = ((opcode & 0xF0) >> 4) * 2 + ((opcode & 0xF) == 0x0E ? 1 : 0);
				pCpu->reg[r1]->R = pCpu->data_bus;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;
			case 0x36: // LD (HL), n
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->address_bus = pCpu->PC + 1;
				cpu_GetByte(pCpu);
				(*byte) = pCpu->data_bus;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			case 0x01: // LD BC, nn
			case 0x11: // LD DE, nn
			case 0x21: // LD HL, nn
			case 0x31: // LD SP, nn
				r1 = ((opcode & 0xF0) >> 4);
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				*pCpu->dreg[r1] = word;
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;

			case 0x02: // LD (BC), A
			case 0x12: // LD (DE), A
				r1 = ((opcode & 0xF0) >> 4);
				pCpu->address_bus = (*pCpu->dreg[r1]);
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->A;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x77: // LD (HL), A
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->A;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xEA: // LD (nn), A
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				pCpu->address_bus = word;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->A;
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;

			case 0x08: // LD nn, SP
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				pCpu->address_bus = word;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->SP;
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;

			case 0x0A: // LD A, (BC)
				pCpu->address_bus = pCpu->BC;
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x1A: // LD A, (DE)
				pCpu->address_bus = pCpu->DE;
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xFA: // LD A, (nn)
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				pCpu->address_bus = word;
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;

			case 0x40: // LD B, B
			case 0x41: // LD B, C
			case 0x42: // LD B, D
			case 0x43: // LD B, E
			case 0x44: // LD B, H
			case 0x45: // LD B, L
			case 0x47: // LD B, A
			case 0x48: // LD C, B
			case 0x49: // LD C, C
			case 0x4A: // LD C, D
			case 0x4B: // LD C, E
			case 0x4C: // LD C, H
			case 0x4D: // LD C, L
			case 0x4F: // LD C, A
			case 0x50: // LD D, B
			case 0x51: // LD D, C
			case 0x52: // LD D, D
			case 0x53: // LD D, E
			case 0x54: // LD D, H
			case 0x55: // LD D, L
			case 0x57: // LD D, A
			case 0x58: // LD E, B
			case 0x59: // LD E, C
			case 0x5A: // LD E, D
			case 0x5B: // LD E, E
			case 0x5C: // LD E, H
			case 0x5D: // LD E, L
			case 0x5F: // LD E, A
			case 0x60: // LD H, B
			case 0x61: // LD H, C
			case 0x62: // LD H, D
			case 0x63: // LD H, E
			case 0x64: // LD H, H
			case 0x65: // LD H, L
			case 0x67: // LD H, A
			case 0x68: // LD L, B
			case 0x69: // LD L, C
			case 0x6A: // LD L, D
			case 0x6B: // LD L, E
			case 0x6C: // LD L, H
			case 0x6D: // LD L, L
			case 0x6F: // LD L, A
			case 0x78: // LD A, B
			case 0x79: // LD A, C
			case 0x7A: // LD A, D
			case 0x7B: // LD A, E
			case 0x7C: // LD A, H
			case 0x7D: // LD A, L
			case 0x7F: // LD A, A
				r1 = (((opcode & 0xF0) >> 4) - 0x04) * 2 + ((opcode & 0xF) > 0x7 ? 1 : 0);
				r2 = (opcode & 0x0F) - ((opcode & 0xF) > 0x7 ? 0x08 : 0);
				pCpu->reg[r1]->R = pCpu->reg[r2]->R;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0x70: // LD (HL), B
			case 0x71: // LD (HL), C
			case 0x72: // LD (HL), D
			case 0x73: // LD (HL), E
			case 0x74: // LD (HL), H
			case 0x75: // LD (HL), L
				r1 = opcode & 0x0F;
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->reg[r1]->R;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0x46: // LD B, (HL)
			case 0x4E: // LD C, (HL)
			case 0x56: // LD D, (HL)
			case 0x5E: // LD E, (HL)
			case 0x66: // LD H, (HL)
			case 0x6E: // LD L, (HL)
			case 0x7E: // LD A, (HL)
				r1 = ((opcode & 0xF0) >> 4) - 0x04 + ((opcode & 0x0F) == 0x0E ? 1 : 0);
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0x22: // LD (HL+), A
			case 0x32: // LD (HL-), A
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->A;
				pCpu->HL += (opcode & 0xF0) == 0x20 ? 1 : -1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x2A: // LD A, (HL+)
			case 0x3A: // LD A, (HL-)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				pCpu->HL += (opcode & 0xF0) == 0x20 ? 1 : -1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0xE0: // LDH ($FF00 + n), A
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->address_bus = 0xFF00 + pCpu->data_bus;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->A;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;
			case 0xE2: // LD (C), A
				pCpu->address_bus = 0xFF00 + pCpu->C;
				byte = cpu_GetByte(pCpu);
				(*byte) = pCpu->A;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0xF0: // LDH A, ($FF00 + n)
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->address_bus = 0xFF00 + pCpu->data_bus;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				break;
			case 0xF2: // LD A, (C)
				pCpu->address_bus = 0xFF00 + pCpu->C;
				byte = cpu_GetByte(pCpu);
				pCpu->A = pCpu->data_bus;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			case 0xF8: // LD HL, SP + n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.Z = 0;
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = (pCpu->SP + pCpu->data_bus) > 0xFFFF;
				pCpu->FLAG_bits.H = ((pCpu->SP & 0xFFF) + pCpu->data_bus) > 0x0FFF;
				pCpu->HL = pCpu->SP + pCpu->data_bus;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			case 0xF9: // LD SP, HL
				pCpu->SP = pCpu->HL;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;


			/* And instructions */
			case 0xA0: // AND B
			case 0xA1: // AND C
			case 0xA2: // AND D
			case 0xA3: // AND E
			case 0xA4: // AND L
			case 0xA5: // AND H
			case 0xA7: // AND A
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 1;
				r1 = opcode & 0x0F;
				pCpu->A &= pCpu->reg[r1]->R;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xA6: // AND (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 1;
				pCpu->A &= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xE6: // AND n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 1;
				pCpu->A &= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;


			/* Xor instructions */
			case 0xA8: // XOR B
			case 0xA9: // XOR C
			case 0xAA: // XOR D
			case 0xAB: // XOR E
			case 0xAC: // XOR H
			case 0xAD: // XOR L
			case 0xAF: // XOR A
				r1 = (opcode & 0x0F) - 0x08;
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->A ^= pCpu->reg[r1]->R;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xAE: // XOR (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->A ^= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xEE: // XOR n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->A ^= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			/* Or instructions */
			case 0xB0: // OR B
			case 0xB1: // OR C
			case 0xB2: // OR D
			case 0xB3: // OR E
			case 0xB4: // OR H
			case 0xB5: // OR L
			case 0xB7: // OR A
				r1 = opcode & 0x0F;
				pCpu->A |= pCpu->reg[r1]->R;
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xB6: // OR (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->A |= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xF6: // OR n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.C = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->A |= pCpu->data_bus;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Compare instructions */
			case 0xB8: // CP B
			case 0xB9: // CP C
			case 0xBA: // CP D
			case 0xBB: // CP E
			case 0xBC: // CP H
			case 0xBD: // CP L
			case 0xBF: // CP A
				r1 = (opcode & 0x0F) - 0x08;
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->reg[r1]->R & 0x0F);
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->reg[r1]->R & 0xF0);
				pCpu->FLAG_bits.Z = pCpu->A == pCpu->reg[r1]->R;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xBE: // CP (HL)
				pCpu->address_bus = pCpu->HL;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->data_bus & 0x0F);
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->data_bus & 0xF0);
				pCpu->FLAG_bits.Z = pCpu->A == pCpu->data_bus;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xFE: // CP n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.H = (pCpu->A & 0x0F) < (pCpu->data_bus & 0x0F);
				pCpu->FLAG_bits.C = (pCpu->A & 0xF0) < (pCpu->data_bus & 0xF0);
				pCpu->FLAG_bits.Z = pCpu->A == pCpu->data_bus;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			/* Rotate Instructions */
			case 0x0F: // RRCA - 9 bit rotate right
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = pCpu->A & 0x01;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.N = 0;
				pCpu->A = (pCpu->A >> 1 & 0x7F) | dummy << 7;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x1F: // RRA
				pCpu->FLAG_bits.C = pCpu->A & 0x01;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.N = 0;
				pCpu->A = (pCpu->A >> 1 & 0x7F) | pCpu->FLAG_bits.C << 7;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x07: // RLCA - 9 bit rotate left
				dummy = pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.C = pCpu->A & 0x80;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.N = 0;
				pCpu->A = (pCpu->A << 1 & 0xFE) | dummy;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x17: // RLA - 8 bit rotate left
				pCpu->FLAG_bits.C = pCpu->A & 0x80;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.N = 0;
				pCpu->A = (pCpu->A << 1 & 0xFE) | pCpu->FLAG_bits.C;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Stop instruction */
			case 0x10: // STOP
				pCpu->stop = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Halt instruction */
			case 0x76:
				pCpu->halt = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Jump relatif instructions */
			case 0x18: // JR n
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				pCpu->PC += 2;
				pCpu->PC += (int8_t)pCpu->data_bus;
				jump = 1;
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;
			case 0x20: // JR NZ
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				if (pCpu->FLAG_bits.Z == 0){
					pCpu->PC += 2;
					pCpu->PC += (int8_t)pCpu->data_bus;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;
			case 0x28: // JR Z
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				if (pCpu->FLAG_bits.Z){
					pCpu->PC += 2;
					pCpu->PC += (int8_t)pCpu->data_bus;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;
			case 0x30: // JR NC
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				if (pCpu->FLAG_bits.C == 0){
					pCpu->PC += 2;
					pCpu->PC += (int8_t)pCpu->data_bus;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;
			case 0x38: // JR C
				pCpu->address_bus = pCpu->PC + 1;
				byte = cpu_GetByte(pCpu);
				if (pCpu->FLAG_bits.C){
					pCpu->PC += 2;
					pCpu->PC += (int8_t)pCpu->data_bus;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, pCpu->data_bus);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, pCpu->data_bus);
				DEBUG_PRINTF("\t");
				break;

			/* Jump absolute instructions */
			case 0xC3: // JP nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				pCpu->PC = word;
				jump = 1;
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xC2: // JP NZ, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.Z == 0){
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xCA: // JP Z, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.Z){
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xD2: // JP NC, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.C == 0){
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xDA: // JP C, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.C){
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xE9: // JP (HL)
				pCpu->PC = pCpu->HL;
				jump = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Call instructions */
			case 0xCD: // CALL nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				cpu_Push(pCpu, pCpu->PC + page0[opcode].size);
				pCpu->PC = word;
				jump = 1;
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xC4: // CALL NZ, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.Z == 0){
					cpu_Push(pCpu, pCpu->PC + page0[opcode].size);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xCC: // CALL Z, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.Z){
					cpu_Push(pCpu, pCpu->PC + page0[opcode].size);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xD4: // CALL NC, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.C == 0){
					cpu_Push(pCpu, pCpu->PC + page0[opcode].size);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;
			case 0xDC: // CALL C, nnnn
				pCpu->address_bus = pCpu->PC + 1;
				word = cpu_GetWordFromPC(pCpu);
				if (pCpu->FLAG_bits.C){
					cpu_Push(pCpu, pCpu->PC + page0[opcode].size);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF(page0[opcode].mnemonic, word);
				DEBUG_PRINTF("\t\t");
				DEBUG_PRINTF(page0[opcode].description, word);
				DEBUG_PRINTF("\t");
				break;

			/* Return instructions */
			case 0xC0: // RET NZ
				if (pCpu->FLAG_bits.Z == 0){
					word = cpu_Pop(pCpu);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xC8: // RET Z
				if (pCpu->FLAG_bits.Z){
					word = cpu_Pop(pCpu);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xC9: // RET
				word = cpu_Pop(pCpu);
				pCpu->PC = word;
				jump = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xD0: // RET NC
				if (pCpu->FLAG_bits.C == 0){
					word = cpu_Pop(pCpu);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xD8: // RET C
				if (pCpu->FLAG_bits.C){
					word = cpu_Pop(pCpu);
					pCpu->PC = word;
					jump = 1;
				}
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xD9: // RETI
				// Restore master interrupt enable flag to its pre-interrupt status
				// TODO RETI
				DEBUG_PRINTF("TODO RETI\n");
				break;

			/* Reset instructions */
			case 0xC7: // RST $0000
			case 0xCF: // RST $0008
			case 0xD7: // RST $0010
			case 0xDF: // RST $0018
			case 0xE7: // RST $0020
			case 0xEF: // RST $0028
			case 0xF7: // RST $0030
			case 0xFF: // RST $0038
				cpu_Push(pCpu, pCpu->PC);
				r1 = (((opcode & 0xF0) >> 4) - 0xC) * 0x10 + ((opcode & 0x0F) == 0x0F ? 0x08 : 0x00);
				pCpu->PC = r1;
				jump = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Pop instructions */
			case 0xC1: // POP BC
			case 0xD1: // POP DE
			case 0xE1: // POP HL
				r1 = ((opcode & 0xF0) >> 4) - 0xC;
				(*pCpu->dreg[r1]) = cpu_Pop(pCpu);
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xF1: // POP AF
				pCpu->AF = cpu_Pop(pCpu);
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Push instructions */
			case 0xC5: // Push BC
			case 0xD5: // Push DE
			case 0xE5: // Push HL
				r1 = ((opcode & 0xF0) >> 4) - 0xC;
				cpu_Push(pCpu, (*pCpu->dreg[r1]));
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xF5: // Push AF
				cpu_Push(pCpu, pCpu->AF);
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Decimal Adjust instruction */
			case 0x27: // DAA
				if (((pCpu->A & 0x0F) > 0x09) || pCpu->FLAG_bits.H)
					pCpu->A += 0x06;
				if (((pCpu->A & 0xF0) > 0x90) || pCpu->FLAG_bits.C){
					pCpu->A += 0x60;
					pCpu->FLAG_bits.C = 1;
				}else{
					pCpu->FLAG_bits.C = 0;
				}
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.Z = pCpu->A == 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Complement instruction */
			case 0x2F: // CPL
				pCpu->A = ~pCpu->A;
				pCpu->FLAG_bits.N = 1;
				pCpu->FLAG_bits.H = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Carry set/reset instructions */
			case 0x37: // SCF
				pCpu->FLAG_bits.C = 1;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0x3F: // CCF
				pCpu->FLAG_bits.C = 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Disable/Enable Interrupt instruction */
			// TODO : check if this is correct
			case 0xF3:
				pCpu->ie_reg->IE = 0;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;
			case 0xFB:
				pCpu->ie_reg->IE = 0xFF;
				DEBUG_PRINTF("%s\t\t%s\t", page0[opcode].mnemonic, page0[opcode].description);
				break;

			/* Illegal instructions */
			case 0xD3:
			case 0xDB:
			case 0xDD:
			case 0xE3:
			case 0xE4:
			case 0xEB:
			case 0xEC:
			case 0xED:
			case 0xF4:
			case 0xFC:
			case 0xFD:
				DEBUG_PRINTF("\nIllegal instruction %02X\n", opcode);
				break;

			default:
				DEBUG_PRINTF("\nUnknown instruction 0x%02Xnot yet implemented.\n", opcode);
				break;
		}
		// increase clock cycle and PC
		pCpu->clock_cycle += page0[opcode].clock_cycles;
		if (!jump && !pCpu->halt && !pCpu->stop)
			pCpu->PC += page0[opcode].size;
	}else{
		DEBUG_PRINTF("$%04X> CB %02X\t%s\t\t%s\t", pCpu->PC, pCpu->data_bus, page1[opcode].mnemonic, page1[opcode].description);
		switch (opcode & 0xF8){
			case 0x00: // RLC 9 bit rotate left with carry
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				dummy = pCpu->FLAG_bits.C;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_7;
					pCpu->reg[r1]->R = dummy | ((pCpu->reg[r1]->R << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x80;
					(*byte) = dummy | ((pCpu->data_bus << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				}
				break;
			case 0x08: // RRC 9 bit rotate right with carry
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				dummy = pCpu->FLAG_bits.C;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_0;
					pCpu->reg[r1]->R = (dummy << 7) | ((pCpu->reg[r1]->R >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					(*byte) = (dummy << 7) | ((pCpu->data_bus >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				}
				break;
			case 0x10: // RL 8 bit rotate left
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_7;
					pCpu->reg[r1]->R = pCpu->FLAG_bits.C | ((pCpu->reg[r1]->R << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x80;
					(*byte) = pCpu->FLAG_bits.C | ((pCpu->data_bus << 1) & 0xFE);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				}
				break;
			case 0x18: // RR 8 bit rotate right
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_0;
					pCpu->reg[r1]->R = (pCpu->FLAG_bits.C << 7) | ((pCpu->reg[r1]->R >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					(*byte) = (pCpu->FLAG_bits.C << 7) | ((pCpu->data_bus >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				}
				break;
			case 0x20: // SLA
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_7;
					pCpu->reg[r1]->R = (pCpu->reg[r1]->R << 1) & 0xFE;
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x80;
					(*byte) = (pCpu->data_bus << 1) & 0xFE;
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				}
				break;
			case 0x28: // SRA
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_0;
					pCpu->reg[r1]->R = (0x80 & pCpu->reg[r1]->R) | ((pCpu->reg[r1]->R >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					(*byte) = (0x80 & pCpu->data_bus) | ((pCpu->data_bus >> 1) & 0x7F);
					pCpu->FLAG_bits.Z = pCpu->data_bus == 0;
				}
				break;
			case 0x30: // SWAP
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;
				pCpu->FLAG_bits.C = 0;

#define SWAP(x) (x) = ((((x) & 0xF0) >> 4) | (((x) & 0x0F) << 4))

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					SWAP(pCpu->reg[r1]->R);
					pCpu->FLAG_bits.Z = !(pCpu->reg[r1]->R);
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					SWAP(pCpu->data_bus);
					pCpu->FLAG_bits.Z = !(pCpu->data_bus);
				}
				break;
			case 0x38: // SRL
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 0;

				r1 = opcode & 0x07;
				if (r1 != 0x06){
					pCpu->FLAG_bits.C = pCpu->reg[r1]->R_bits.bit_0;
					pCpu->reg[r1]->R = 0x7F & (pCpu->reg[r1]->R >> 1);
					pCpu->FLAG_bits.Z = pCpu->reg[r1]->R == 0;
				}else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					pCpu->FLAG_bits.C = pCpu->data_bus & 0x01;
					(*byte) = 0x7F & (pCpu->data_bus >> 1);
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
				pCpu->FLAG_bits.N = 0;
				pCpu->FLAG_bits.H = 1;
				bit = (((opcode & 0xF0) >> 4) - 0x4) * 2;
				bit = bit + ((opcode & 0x08) ? 1 : 0);
				mask = 1 << bit;
				r1 = opcode & 0x07;
				if (r1 != 0x06)
					pCpu->FLAG_bits.Z = !(pCpu->reg[r1]->R & mask);
				else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
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
				bit = (((opcode & 0xF0) >> 4) - 0x8) * 2;
				bit = bit + ((opcode & 0x08) ? 1 : 0);
				mask = 1 << bit;
				r1 = opcode & 0x07;
				if (r1 != 0x06)
					pCpu->reg[r1]->R &= ~mask;
				else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("\ncpu_GetByte failed\n");
						return;
					}
					(*byte) = pCpu->data_bus & ~mask;
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
				bit = (((opcode & 0xF0) >> 4) - 0xC) * 2;
				bit = bit + ((opcode & 0x08) ? 1 : 0);
				mask = 1 << bit;
				r1 = opcode & 0x07;
				if (r1 != 0x06)
					pCpu->reg[r1]->R |= mask;
				else{
					pCpu->address_bus = pCpu->HL;
					byte = cpu_GetByte(pCpu);
					if (byte == NULL){
						DEBUG_PRINTF("cpu_GetByte failed\n");
						return;
					}
					(*byte) = pCpu->data_bus | mask;
				}
				break;
			default:
				DEBUG_PRINTF("\nunknown instruction #%X\n", opcode);
				break;
		}
		// reset extended mode
		pCpu->extended = 0;
		// increase clock cycle and PC
		pCpu->clock_cycle += page1[opcode].clock_cycles;
		pCpu->PC += page1[opcode].size;
	}

	DEBUG_PRINTF("%02X | %02X | %02X | %02X |", pCpu->A, pCpu->B, pCpu->C, pCpu->D);
	DEBUG_PRINTF("%02X | %02X | %02X | ", pCpu->E, pCpu->H, pCpu->L);
	DEBUG_PRINTF("%c%c", pCpu->FLAG_bits.C ? 'C': 'c', pCpu->FLAG_bits.H ? 'H': 'h');
	DEBUG_PRINTF("%c%c", pCpu->FLAG_bits.N ? 'N': 'n', pCpu->FLAG_bits.Z ? 'Z': 'z');
	DEBUG_PRINTF("\n");
}
