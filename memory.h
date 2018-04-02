#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Memory structure
typedef struct{
	uint8_t *data; // pointer to allocated memory
	uint32_t size; // size of memory
	uint32_t banks; // number of banks if the memory can be accessed through a banking system
	uint32_t bank_size; // size of each bank
	uint32_t start_idx; // start idx of data, used as offset for banking
}Memory;

// Initialize and return a Memory structure
Memory *mem_Init(uint32_t size, uint32_t banks, uint32_t bank_size);
// Free a memory
void mem_Free(Memory *pMem);

// Set start index of a memory
void mem_SetStartIndex(Memory *pMem, uint32_t start_idx);
// Copy all data from one memory to the other, used for mapping
void mem_CopyInfo(Memory *pDest, Memory *pSrc);

// Read from memory at a given address
uint16_t mem_Read(Memory *pMem, uint32_t address);
// Read multiple bytes at a given address
uint8_t mem_ReadMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t read_size);
// Write byte in memory to address
uint16_t mem_Write(Memory *pMem, uint32_t address, uint8_t data);
// Write multiple bytes in memory at address
uint8_t mem_WriteMulti(Memory *pMem, uint32_t address, uint8_t *data, uint32_t write_size);


#endif
