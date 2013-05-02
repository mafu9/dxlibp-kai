#include "../mutex.h"

int CreateMutex(const char *mutexname)
{
	DXPMUTEX *pHnd;
	int ret, i;
	if(dxpMutexLock(&dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX], FALSE) < 0)return -1;
	for(i = 0; i < DXP_MUTEXHANDLE_MAX; ++i)
		if(dxpMutexData.handleArray[i].semaid < 0)break;
	if(i >= DXP_MUTEXHANDLE_MAX)
	{
		dxpMutexUnlock(&dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX]);
		return -1;
	}
	pHnd = &dxpMutexData.handleArray[i];
	pHnd->semaid = sceKernelCreateSema(mutexname, 0, 1, 1, NULL);
	if(pHnd->semaid < 0)
	{
		ret = -1;
	}
	else
	{
		pHnd->ownerThid = -1;
		pHnd->count = 0;
		ret = i;
	}
	dxpMutexUnlock(&dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX]);
	return ret;
}

