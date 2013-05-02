#include "../fileio.h"

int	FileRead_seek(int filehandle,int offset,int origin)
{
	int res, ret;
	DXPFILEIOHANDLE *pHnd;
	if(origin < 0 || origin > 2)return -1;
	if(!dxpFileioData.init)return -1;
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
			break;
		default:
			FCRITICALSECTION_UNLOCK(filehandle);
			return -1;
		}
		target += offset;
		if(target < 0)target = 0;
		if(target > pHnd->size)target = pHnd->size;
		pHnd->pos = target;
		ret = 0;
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
		res = sceIoLseek32(pHnd->fd,offset,origin);
		if(res < 0)
		{
			ret = -1;
		}
		else
		{
			pHnd->pos = res;
			ret = 0;
		}
	}
	FCRITICALSECTION_UNLOCK(filehandle);
	return ret;
}
