#include "../fileio.h"

int FileRead_eof(int filehandle)
{
	DXPFILEIOHANDLE *pHnd;
	FHANDLE2PTR(pHnd,filehandle);
	if(pHnd->size <= 0)return 1;
	if(pHnd->pos >= pHnd->size)
	{
		pHnd->pos = pHnd->size;
		return 1;
	}
	return 0;
}