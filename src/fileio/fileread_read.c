#include "../fileio.h"

int	FileRead_read(void *buffer,int readsize,int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		int i;
		for(i = 0;i < readsize && pHnd->pos < pHnd->size;++i,++pHnd->pos)
			((u8*)buffer)[pHnd->pos] = ((u8*)pHnd->dat)[pHnd->pos];
		return i;
	}
	if(dxpFileioData.sleep)
		if(dxpFileioReopen(filehandle) < 0)return -1;
	int retread = sceIoRead(pHnd->fd,buffer,readsize);
	pHnd->pos += retread;
	return retread;
}