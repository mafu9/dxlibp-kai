#include "../mutex.h"

int LockMutex(int mutexhandle)
{
	DXPMUTEX *pHnd;
	MHANDLE2PTR(pHnd, mutexhandle);
	return dxpMutexLock(pHnd, FALSE);
}

