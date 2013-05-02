#include "../fileio.h"

int	FileRead_read(void *buffer,int readsize,int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	int retread;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		int i;
		for(i = 0;i < readsize && pHnd->pos < pHnd->size;++i,++pHnd->pos)
			((u8*)buffer)[pHnd->pos] = ((const u8*)pHnd->dat)[pHnd->pos];
		retread = i;
	}
	else
	{
		if(dxpFileioData.sleep)
		{
			if(dxpFileioReopen(pHnd) < 0)
			{
				FCRITICALSECTION_UNLOCK(filehandle);
				return -1;
			}
		}
		retread = sceIoRead(pHnd->fd,buffer,readsize);
		pHnd->pos += retread;
	}
	FCRITICALSECTION_UNLOCK(filehandle);
	return retread;
}