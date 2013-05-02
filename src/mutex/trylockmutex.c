#include "../mutex.h"

int TryLockMutex(int mutexhandle)
{
	DXPMUTEX *pHnd;
	MHANDLE2PTR(pHnd, mutexhandle);
	return dxpMutexLock(pHnd, TRUE);
}

