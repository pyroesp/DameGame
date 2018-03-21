#ifndef _RAM_H
#define _RAM_H

#include <stdint.h>

#define RAM_SIZE (0x8000)
#define RAM_BANK_SIZE (0x2000)

#define RAM_BANK_0 (0)
#define RAM_BANK_1 (1)
#define RAM_BANK_2 (2)
#define RAM_BANK_3 (3)
#define RAM_TOTAL_BANK (4)

#define RAM_BANK_0_OFFSET (0x0000)
#define RAM_BANK_1_OFFSET (0x2000)
#define RAM_BANK_2_OFFSET (0x4000)
#define RAM_BANK_3_OFFSET (0x6000)

#endif
