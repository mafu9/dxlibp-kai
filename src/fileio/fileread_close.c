#include "../fileio.h"
#include <pspsdk.h>

int FileRead_close(int filehandle)
{
	//int istate;
	DXPFILEIOHANDLE *pHnd;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	//istate = pspSdkDisableInterrupts();
	if(!pHnd->onmemory)sceIoClose(pHnd->fd);
	pHnd->used = 0;
	FCRITICALSECTION_UNLOCK(filehandle);
	//pspSdkEnableInterrupts(istate);
	return 0;
}