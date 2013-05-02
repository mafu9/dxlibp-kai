#include "../fileio.h"
#include <psppower.h>
#include <unistd.h>
#include <string.h>
//variables ----

DXPFILEIODATA dxpFileioData = 
{
	.init = 0,
	.sleep = 0,
};

//local functions ----


static int dxpPowerCallback(int unk0,int flag,void* arg)
{
	if(flag & PSP_POWER_CB_SUSPENDING)
	{
		dxpFileioData.sleep = 1;
	}
	if(flag & PSP_POWER_CB_RESUME_COMPLETE)
	{
		dxpFileioData.sleep = 0;
	}
	return 0;
}

static int dxpPowerCallbackThread(SceSize args, void *argp)
{
	SceUID cbid;
	cbid = sceKernelCreateCallback("dxp power callback", dxpPowerCallback, NULL);
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
		dxpFileioData.eventFlags[i] = sceKernelCreateEventFlag(name, 0, 0xFFFFFFFF, NULL);
		if(dxpFileioData.eventFlags[i] < 0)return -1;
	}
	dxpPowerSetupCallback();
	dxpFileioData.init = 1;
	return 0;
}

int dxpFileioReopen(DXPFILEIOHANDLE *pHnd)
{
	char name[DXP_BUILDOPTION_FILENAMELENGTH_MAX];
	if(!dxpFileioData.init)return -1;
	if(!pHnd)return -1;
	if(pHnd->onmemory)return 0;
	while(dxpFileioData.sleep)sceKernelDelayThread(100);
	pHnd->fd = sceIoOpen(pHnd->filename,PSP_O_RDONLY,0777);
	if(pHnd->fd == SCE_KERNEL_ERROR_NOCWD)
	{
		getcwd(name,DXP_BUILDOPTION_FILENAMELENGTH_MAX);
		int len = strlen(name);
		if(len >= DXP_BUILDOPTION_FILENAMELENGTH_MAX)return -1;
		name[len] = '/';
		strncpy(name + len + 1,pHnd->filename,DXP_BUILDOPTION_FILENAMELENGTH_MAX - len - 1);
		pHnd->fd = sceIoOpen(name,PSP_O_RDONLY,0777);
		if(pHnd->fd >= 0)strncpy(pHnd->filename,name,DXP_BUILDOPTION_FILENAMELENGTH_MAX);
	}
	if(pHnd->fd < 0)return -1;
	sceIoLseek32(pHnd->fd,pHnd->pos,SEEK_SET);
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

