#include "../fileio.h"

int FileRead_tell(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	FHANDLE2PTR(pHnd,filehandle);
	return pHnd->pos;
}