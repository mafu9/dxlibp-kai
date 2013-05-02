#include "../mutex.h"

int GetMutexCount(int mutexhandle)
{
	DXPMUTEX *pHnd;
	MHANDLE2PTR(pHnd, mutexhandle);
	return pHnd->count;
}

