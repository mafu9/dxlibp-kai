#include "../fileio.h"

int	FileRead_seek(int filehandle,int offset,int origin)
{
	int res;
	DXPFILEIOHANDLE *pHnd;
	if(origin < 0 || origin > 2)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->onmemory)
	{
		int target = 0;
		switch(origin)
		{
		case SEEK_CUR:
			target = pHnd->pos;
			break;
		case SEEK_SET:
			target = 0;
			break;
		case SEEK_END:
			target = pHnd->size;
		}
		target += offset;
		if(target < 0)target = 0;
		if(target > pHnd->size)target = pHnd->size;
		pHnd->pos = target;
		return 0;
	}
	if(dxpFileioData.sleep)
		if(dxpFileioReopen(filehandle) < 0)return -1;
	res = sceIoLseek32(pHnd->fd,offset,origin);
	if(res < 0)
		return -1;
	pHnd->pos = res;
	return 0;
}
