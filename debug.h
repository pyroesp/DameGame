#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>

#define DEBUG_ENABLE

#if defined(DEBUG_ENABLE)
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#endif
