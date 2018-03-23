#include "memory.h"

Memory *mem_Init(uint32_t size, uint32_t banks, uint32_t bank_size){
	Memory *mem = NULL;

	if ((size/banks == bank_size) && (size%banks == 0)){
		mem = (Memory*)malloc(sizeof(Memory));
		mem->size = size;
		mem->banks = banks;
		mem->bank_size = bank_size;

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
}


uint16_t mem_Read(Memory *pMem, uint32_t address){
	if (address < pMem->size)
		return pMem->data[address];
	return 0x100; // error for now is 256
}

uint8_t mem_ReadMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t read_size){
	if (address + read_size < pMem->size){
		memcpy(data, &pMem->data[address], sizeof(uint8_t) * read_size);
		return 1;
	}
	return 0;
}

uint16_t mem_Write(Memory *pMem, uint32_t address, uint8_t data){
	if (address < pMem->size){
		pMem->data[address] = data;
		return data;
	}
	return 0x100; // error for now is 256
}

uint8_t mem_WriteMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t write_size){
	if (address + write_size < pMem->size){
		memcpy(&pMem->data[address], data, sizeof(uint8_t) * write_size);
		return 1;
	}
	return 0;
}
