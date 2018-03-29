#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#if defined(DEBUG_ENABLE)
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#endif
