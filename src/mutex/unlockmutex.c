#include "../mutex.h"

int UnlockMutex(int mutexhandle)
{
	DXPMUTEX *pHnd;
	MHANDLE2PTR(pHnd, mutexhandle);
	return dxpMutexUnlock(pHnd);
}

