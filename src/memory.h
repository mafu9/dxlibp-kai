#include "dxlibp.h"
#include <malloc.h>


//structures ----

typedef struct DXPMEMORYDATA
{
	unsigned init : 1;
	unsigned memAssert : 1;
	SceUID mutex;
	SceUID mutexOwnerThid;
	int mutexCount;
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
void dxpMemoryLock(void);
void dxpMemoryUnlock(void);


//variables ----

extern DXPMEMORYDATA dxpMemoryData;

