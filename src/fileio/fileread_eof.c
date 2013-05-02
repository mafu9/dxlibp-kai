#include "../fileio.h"

int FileRead_eof(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	if(!dxpFileioData.init)return -1;
	FCRITICALSECTION_LOCK(filehandle);
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->size <= 0)return 1;
	if(pHnd->pos >= pHnd->size)
	{
		pHnd->pos = pHnd->size;
		return 1;
	}
	return 0;
}