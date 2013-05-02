#include "../fileio.h"

int FileRead_tell(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	int pos;
	if(!dxpFileioData.init)return -1;
	FHANDLE2PTR(pHnd,filehandle);
	pos = pHnd->pos;
	FCRITICALSECTION_UNLOCK(filehandle);
	return pos;
}