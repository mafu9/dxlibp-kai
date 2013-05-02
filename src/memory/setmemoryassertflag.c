#include "../mutex.h"
#include "../memory.h"

int SetMemoryAssertFlag(int flag)
{
	LockMutex(dxpMemoryData.mutexHandle);
	dxpMemoryData.memAssert = flag;
	UnlockMutex(dxpMemoryData.mutexHandle);
	return 0;
}

