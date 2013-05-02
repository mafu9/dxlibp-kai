#include "dxlibp.h"
#include <malloc.h>


//structures ----

typedef struct DXPMEMORYDATA
{
	unsigned init : 1;
	unsigned memAssert : 1;
	int mutexHandle;
	unsigned int totalMemSize;
} DXPMEMORYDATA;

typedef struct DXPMEMORYLIST
{
	void *buf;
	struct DXPMEMORYLIST *next;
} DXPMEMORYLIST;


//functions ----

int dxpMemoryInit(void);
int dxpMemoryEnd(void);


//variables ----

extern DXPMEMORYDATA dxpMemoryData;

