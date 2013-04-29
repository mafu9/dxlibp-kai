#include "../fileio.h"
#include <pspsdk.h>

int FileRead_close(int filehandle)
{
	//int istate;
	DXPFILEIOHANDLE *pHnd;
	FHANDLE2PTR(pHnd,filehandle);
	//istate = pspSdkDisableInterrupts();
	if(!pHnd->onmemory)
	{
		sceIoClose(pHnd->fd);
	}
	pHnd->used = 0;
	//pspSdkEnableInterrupts(istate);
	return 0;
}