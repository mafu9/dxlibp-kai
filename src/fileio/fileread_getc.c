#include "../fileio.h"
#include <stdio.h>
int FileRead_getc(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		if(pHnd->pos >= pHnd->size)
		{
			pHnd->pos = pHnd->size;
			return -1;
		}
		return ((u8*)pHnd->dat)[pHnd->pos++];
	}
	if(dxpFileioData.sleep)
		if(dxpFileioReopen(filehandle) < 0)return -1;

	char c;
	int status;
	status = sceIoRead(dxpFileioData.handleArray[filehandle].fd,&c,1);
	if(status != 1)return -1;
	pHnd->pos += 1;
	return c;
}