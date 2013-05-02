#include "../mutex.h"

DXPMUTEXDATA dxpMutexData;

int dxpMutexInit(void)
{
	int i;
	for(i = 0; i < DXP_MUTEXHANDLE_MAX; ++i)dxpMutexData.handleArray[i].semaid = -1;
	dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX].semaid = sceKernelCreateSema("dxp mutex sema", 0, 1, 1, NULL);
	if(dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX].semaid < 0)return -1;
	dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX].ownerThid = -1;
	dxpMutexData.handleArray[DXP_MUTEX_OWN_INDEX].count = 0;
	return 0;
}

int dxpMutexLock(DXPMUTEX *pHnd, int tryFlag)
{
	int ret;
	const SceUID thid = sceKernelGetThreadId();
	if(thid != pHnd->ownerThid)
	{
		SceUInt timeout = 0;
		ret = sceKernelWaitSema(pHnd->semaid, 1, tryFlag ? &timeout : NULL);
		pHnd->ownerThid = thid;
	}
	else
	{
		pHnd->count++;
		ret = 0;
	}
	return ret;
}

int dxpMutexUnlock(DXPMUTEX *pHnd)
{
	int ret;
	if(pHnd->count > 0)
	{
		pHnd->count--;
		ret = 0;
	}
	else
	{
		pHnd->ownerThid = -1;
		ret = sceKernelSignalSema(pHnd->semaid, 1);
	}
	return ret;
}

