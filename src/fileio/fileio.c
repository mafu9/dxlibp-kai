#include "../mutex.h"
#include "../fileio.h"
#include <psppower.h>
#include <unistd.h>
#include <string.h>
//variables ----

DXPFILEIODATA dxpFileioData = 
{
	.init = 0,
};

//local functions ----


static int dxpPowerCallback(int unk0,int flag,void* arg)
{
	int *suspend = (int*)arg;
	int i;
	if(flag & PSP_POWER_CB_SUSPENDING)
	{
		if(!(*suspend))
		{
			if(LockMutex(dxpFileioData.mutexHandle) >= 0)
			{
				for(i = 0; i < DXP_SCE_IO_HANDLE_MAX; ++i)
				{
					if(dxpFileioData.sceIoActiveData[i].fd >= 0)
					{
						sceIoClose(dxpFileioData.sceIoActiveData[i].fd);
						dxpFileioData.sceIoActiveData[i].pHnd = NULL;
						dxpFileioData.sceIoActiveData[i].fd = -1;
					}
				}
				*suspend = TRUE;
			}
		}
	}
	if(flag & PSP_POWER_CB_RESUME_COMPLETE)
	{
		if(*suspend)
		{
			if(UnlockMutex(dxpFileioData.mutexHandle) >= 0)
				*suspend = FALSE;
		}
	}
	sceKernelDelayThread(1000000);
	return 0;
}

static int dxpPowerCallbackThread(SceSize args, void *argp)
{
	SceUID cbid;
	int suspend = 0;
	cbid = sceKernelCreateCallback("dxp power callback", dxpPowerCallback, &suspend);
	scePowerRegisterCallback(-1, cbid);
	sceKernelSleepThreadCB();
	return 0;
}

static int dxpPowerSetupCallback(void)
{
	SceUID thid = 0;
	thid = sceKernelCreateThread("update_thread", dxpPowerCallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)sceKernelStartThread(thid, 0, 0);
	return thid;
}

int dxpFileioInit(void)
{
	char name[32];
	int i;
	if(dxpFileioData.init)return 0;
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)dxpFileioData.handleArray[i].used = 0;
	for(i = 0; i < sizeof(dxpFileioData.eventFlags) / sizeof(dxpFileioData.eventFlags[0]); ++i)
	{
		snprintf(name, 32, "dxp file event flag %d", i);
		name[31] = '\0';
		dxpFileioData.eventFlags[i] = sceKernelCreateEventFlag(name, 0, 0xFFFFFFFF, NULL);
		if(dxpFileioData.eventFlags[i] < 0)return -1;
	}
	dxpFileioData.mutexHandle = CreateMutex("fileio mutex");
	if(dxpFileioData.mutexHandle < 0)return -1;
	for(i = 0; i < DXP_SCE_IO_HANDLE_MAX; ++i)
	{
		dxpFileioData.sceIoActiveData[i].pHnd = NULL;
		dxpFileioData.sceIoActiveData[i].fd = -1;
	}
	dxpFileioData.sceIoActiveDataTail = 0;
	dxpPowerSetupCallback();
	dxpFileioData.init = 1;
	return 0;
}

int dxpFileioOpenOnMemory(const void *buffer, u32 size)
{
	DXPFILEIOHANDLE *pHnd;
	int i;
	if(!dxpFileioData.init)return 0;
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)
	{
		if(!dxpFileioData.handleArray[i].used)
		{
			FCRITICALSECTION_LOCK(i + 1);
			if(!dxpFileioData.handleArray[i].used)break;
			FCRITICALSECTION_UNLOCK(i + 1);
		}
	}
	if(i >= DXP_BUILDOPTION_FILEHANDLE_MAX)return 0;
	pHnd = &dxpFileioData.handleArray[i];
	strcpy(pHnd->filename, "@mem");
	pHnd->used = 1;
	pHnd->onmemory = 1;
	pHnd->pos = 0;
	pHnd->dat = buffer;
	pHnd->size = size;
	FCRITICALSECTION_UNLOCK(i + 1);
	return i + 1;
}

int dxpSceIoReopen(DXPFILEIOHANDLE *pHnd)
{
	SceUID fd;
	char name[DXP_BUILDOPTION_FILENAMELENGTH_MAX];
	if(!dxpFileioData.init)return -1;
	if(!pHnd)return -1;
	if(pHnd->onmemory)return 0;
	if(LockMutex(dxpFileioData.mutexHandle) < 0)return -1;
	fd = dxpSceIoFindFd(pHnd);
	if(fd < 0)
	{
		fd = sceIoOpen(pHnd->filename, PSP_O_RDONLY, 0777);
		if(fd == SCE_KERNEL_ERROR_NOCWD)
		{
			getcwd(name,DXP_BUILDOPTION_FILENAMELENGTH_MAX);
			int len = strlen(name);
			if(len < DXP_BUILDOPTION_FILENAMELENGTH_MAX)
			{
				name[len] = '/';
				strncpy(name + len + 1,pHnd->filename,DXP_BUILDOPTION_FILENAMELENGTH_MAX - len - 1);
				fd = sceIoOpen(name,PSP_O_RDONLY,0777);
				if(fd >= 0)strncpy(pHnd->filename,name,DXP_BUILDOPTION_FILENAMELENGTH_MAX);
			}
		}
		if(fd >= 0)
		{
			sceIoLseek32(fd,pHnd->pos,SEEK_SET);
			dxpSceIoPushBack(pHnd, fd);
		}
	}
	if(UnlockMutex(dxpFileioData.mutexHandle) < 0)return -1;
	return fd;
}

SceUID dxpSceIoFindFd(DXPFILEIOHANDLE *pHnd)
{
	SceUID fd = -1;
	int i;
	if(LockMutex(dxpFileioData.mutexHandle) < 0)return -1;
	for(i = 0; i < DXP_SCE_IO_HANDLE_MAX; ++i)
	{
		if(dxpFileioData.sceIoActiveData[i].pHnd == pHnd)
		{
			fd = dxpFileioData.sceIoActiveData[i].fd;
			break;
		}
	}
	if(UnlockMutex(dxpFileioData.mutexHandle) < 0)return -1;
	return fd;
}

void dxpSceIoPushBack(DXPFILEIOHANDLE *pHnd, SceUID fd)
{
	if(LockMutex(dxpFileioData.mutexHandle) < 0)return;

	if(dxpFileioData.sceIoActiveData[dxpFileioData.sceIoActiveDataTail].fd >= 0 )
		sceIoClose(dxpFileioData.sceIoActiveData[dxpFileioData.sceIoActiveDataTail].fd);

	dxpFileioData.sceIoActiveData[dxpFileioData.sceIoActiveDataTail].pHnd = pHnd;
	dxpFileioData.sceIoActiveData[dxpFileioData.sceIoActiveDataTail].fd = fd;

	dxpFileioData.sceIoActiveDataTail++;
	if(dxpFileioData.sceIoActiveDataTail >= DXP_SCE_IO_HANDLE_MAX)
		dxpFileioData.sceIoActiveDataTail = 0;

	UnlockMutex(dxpFileioData.mutexHandle);
}

void dxpSceIoErase(DXPFILEIOHANDLE *pHnd)
{
	int i;
	if(LockMutex(dxpFileioData.mutexHandle) < 0)return;
	for(i = 0; i < DXP_SCE_IO_HANDLE_MAX; ++i)
	{
		if(dxpFileioData.sceIoActiveData[i].pHnd == pHnd)
		{
			if(dxpFileioData.sceIoActiveData[i].fd >= 0)
				sceIoClose(dxpFileioData.sceIoActiveData[i].fd);
			dxpFileioData.sceIoActiveData[i].pHnd = NULL;
			dxpFileioData.sceIoActiveData[i].fd = -1;
			break;
		}
	}
	UnlockMutex(dxpFileioData.mutexHandle);
}

