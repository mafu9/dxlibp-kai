#include "../fileio.h"
#include <pspsdk.h>

int FileRead_close(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	if(!pHnd->onmemory)dxpSceIoErase(pHnd);
	pHnd->used = 0;
	FCRITICALSECTION_UNLOCK(filehandle);
	return 0;
}