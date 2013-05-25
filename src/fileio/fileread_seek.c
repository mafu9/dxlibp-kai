#include "../fileio.h"

int	FileRead_seek(int filehandle,int offset,int origin)
{
	int ret = 0;
	DXPFILEIOHANDLE *pHnd;
	if(origin < 0 || origin > 2)return -1;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	switch(origin)
	{
	case SEEK_CUR:
		offset += pHnd->pos;
		break;
	case SEEK_SET:
		break;
	case SEEK_END:
		offset+= pHnd->size;
		break;
	default:
		FCRITICALSECTION_UNLOCK(filehandle);
		return -1;
	}
	if(offset < 0)offset = 0;
	if(offset > pHnd->size)offset = pHnd->size;
	if(!pHnd->onmemory && pHnd->pos != offset)
	{
		SceUID fd = dxpSceIoFindFd(pHnd);
		if(fd >= 0)
		{
			if(sceIoLseek32(fd,offset,SEEK_SET) < 0)ret = -1;
		}
	}
	pHnd->pos = offset;
	FCRITICALSECTION_UNLOCK(filehandle);
	return ret;
}
