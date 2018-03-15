#include <stdint.h>
#include <stdio.h>

#include "cpu.h"

int main(int argc, char *argv[]){
    int i;
	uint8_t mem[MEM_TOTAL] = {0};

	mem[1] = 0xCB;
	mem[2] = 0x88; // clear bit 1 of reg B
	mem[3] = 0xCB;
	mem[4] = 0x80; // clear bit 0 of reg B

	Cpu *cpu = NULL;
	cpu = cpu_Init(mem);

	for (i = 0; i < 5; i++)
        printf("%02X  ", mem[i]);
    printf("\n");

    cpu->B = 0xFF;
	cpu_Execute_Opcode(cpu, mem);
	cpu_Execute_Opcode(cpu, mem);
	cpu_Execute_Opcode(cpu, mem);

	printf("cpu->B = %02X\n", cpu->B); // result should be 0xFD

	cpu_Free(cpu);
	cpu = NULL;
	return 0;
}
