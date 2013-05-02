#include "../fileio.h"

int FileRead_eof(int filehandle)
{
	int ret;
	DXPFILEIOHANDLE *pHnd;
	if(!dxpFileioData.init)return -1;
	FCRITICALSECTION_LOCK(filehandle);
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->size <= 0)return 1;
	if(pHnd->pos >= pHnd->size)
	{
		pHnd->pos = pHnd->size;
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	FCRITICALSECTION_UNLOCK(filehandle);
	return ret;
}