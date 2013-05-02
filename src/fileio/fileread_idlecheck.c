#include "../fileio.h"

int FileRead_idle_chk(int filehandle)
{
	if(!dxpFileioData.init)return -1;
//	FHANDLE2PTR(pHnd,filehandle);
//	FCRITICALSECTION_UNLOCK(filehandle);
	return 1;
}