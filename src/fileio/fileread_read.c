#include "../fileio.h"

int	FileRead_read(void *buffer,int readsize,int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	int retread = -1;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		int i;
		for(i = 0;i < readsize && pHnd->pos < pHnd->size;++i,++pHnd->pos)
			((u8*)buffer)[i] = ((const u8*)pHnd->dat)[pHnd->pos];
		retread = i;
	}
	else
	{
		SceUID fd = dxpSceIoReopen(pHnd);
		if(fd >= 0)
		{
			retread = sceIoRead(fd,buffer,readsize);
			pHnd->pos += retread;
		}
	}
	FCRITICALSECTION_UNLOCK(filehandle);
	return retread;
}