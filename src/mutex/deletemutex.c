#include "../mutex.h"

int DeleteMutex(int mutexhandle)
{
	DXPMUTEX *pHnd;
	MHANDLE2PTR(pHnd, mutexhandle);
	if(!pHnd->semaid < 0)return -1;
	if(sceKernelDeleteSema(pHnd->semaid) < 0)return -1;
	pHnd->semaid = -1;
	return 0;
}

