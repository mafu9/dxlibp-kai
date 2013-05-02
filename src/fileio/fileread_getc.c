#include "../fileio.h"
#include <stdio.h>
int FileRead_getc(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	int ret = -1;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		int ret;
		if(pHnd->pos >= pHnd->size)
		{
			pHnd->pos = pHnd->size;
			ret = -1;
		}
		else
		{
			ret = (int)((const u8*)pHnd->dat)[pHnd->pos++];
		}
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
		char c;
		int status;
		status = sceIoRead(dxpFileioData.handleArray[filehandle].fd,&c,1);
		if(status != 1)
		{
			ret = -1;
		}
		else
		{
			pHnd->pos += 1;
			ret = (int)c;
		}
	}
	FCRITICALSECTION_UNLOCK(filehandle);
	return ret;
}