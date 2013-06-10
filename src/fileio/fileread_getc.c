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
		if(pHnd->pos >= pHnd->size)
			pHnd->pos = pHnd->size;
		else
			ret = (int)((const u8*)pHnd->dat)[pHnd->pos++];
	}
	else
	{
		SceUID fd = dxpSceIoReopen(pHnd);
		if(fd >= 0)
		{
			char c;
			if(sceIoRead(fd,&c,1) == 1)
			{
				pHnd->pos += 1;
				ret = (int)c;
			}
		}
	}
	FCRITICALSECTION_UNLOCK(filehandle);
	return ret;
}