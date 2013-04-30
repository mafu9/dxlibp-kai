#include "../memory.h"

int SetMemoryAssertFlag(int flag)
{
	dxpMemoryLock();
	dxpMemoryData.memAssert = flag;
	dxpMemoryUnlock();
	return 0;
}

