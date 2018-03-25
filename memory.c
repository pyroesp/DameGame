#include "memory.h"

Memory *mem_Init(uint32_t size, uint32_t banks, uint32_t bank_size){
	Memory *mem = NULL;

	if ((size/banks == bank_size) && (size%banks == 0)){
		mem = (Memory*)malloc(sizeof(Memory));
		mem_SetSize(mem,size);
		mem_SetBanks(mem, banks);
		mem_SetBanks(mem, bank_size);
		mem_SetStartIndex(mem, 0);
		mem->data = (uint8_t*)malloc(sizeof(uint8_t) * size);

		return mem;
	}

	return NULL;
}

void mem_Free(Memory *pMem){
	free(pMem->data);
	pMem->data = NULL;
	free(pMem);
	pMem = NULL;
	return;
}

void mem_SetStartIndex(Memory *pMem, uint32_t start_idx){
	pMem->start_idx = start_idx;
	return;
}

void mem_CopyInfo(Memory *pDest, Memory *pSrc){
	pDest->data = pSrc->data;
	pDest->banks = pSrc->banks;
	pDest->bank_size = pSrc->bank_size;
	pDest->start_idx = pSrc->start_idx;
	return;
}

uint16_t mem_Read(Memory *pMem, uint32_t address){
	if (pMem->start_idx + address < pMem->size)
		return pMem->data[pMem->start_idx + address];
	return 0x100; // error for now is 256
}

uint8_t mem_ReadMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t read_size){
	if (pMem->start_idx + address + read_size < pMem->size){
		memcpy(data, &pMem->data[pMem->start_idx + address], sizeof(uint8_t) * read_size);
		return 1;
	}
	return 0;
}

uint16_t mem_Write(Memory *pMem, uint32_t address, uint8_t data){
	if (pMem->start_idx + address < pMem->size){
		pMem->data[pMem->start_idx + address] = data;
		return data;
	}
	return 0x100; // error for now is 256
}

uint8_t mem_WriteMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t write_size){
	if (pMem->start_idx + address + write_size < pMem->size){
		memcpy(&pMem->data[pMem->start_idx + address], data, sizeof(uint8_t) * write_size);
		return 1;
	}
	return 0;
}
