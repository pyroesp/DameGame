#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct{
	uint8_t *data;
	uint32_t size;
	uint32_t banks;
	uint32_t bank_size;
	uint32_t start_idx;
}Memory;


Memory *mem_Init(uint32_t size, uint32_t banks, uint32_t bank_size);
void mem_Free(Memory *pMem);

void mem_SetStartIndex(Memory *pMem, uint32_t start_idx);
void mem_CopyInfo(Memory *pDest, Memory *pSrc);

uint16_t mem_Read(Memory *pMem, uint32_t address);
uint8_t mem_ReadMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t read_size);
uint16_t mem_Write(Memory *pMem, uint32_t address, uint8_t data);
uint8_t mem_WriteMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t write_size);


#endif
