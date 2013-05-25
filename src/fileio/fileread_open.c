#include "../fileio.h"
#include <string.h>
#include <pspkerror.h>
#include <unistd.h>
#include <pspsdk.h>
int FileRead_open(const char* filename,int async)
{
	DXPFILEIOHANDLE *pHnd;
	int ret, i;
	if(!filename)return 0;
	if(strlen(filename) + 1 >= DXP_BUILDOPTION_FILENAMELENGTH_MAX)return 0;
	if(!dxpFileioData.init)return 0;
	for(i = 0;i < DXP_BUILDOPTION_FILEHANDLE_MAX;++i)
	{
		if(!dxpFileioData.handleArray[i].used)
		{
			FCRITICALSECTION_LOCK(i + 1);
			if(!dxpFileioData.handleArray[i].used)break;
			FCRITICALSECTION_UNLOCK(i + 1);
		}
	}
	if(i >= DXP_BUILDOPTION_FILEHANDLE_MAX)return 0;
	pHnd = &dxpFileioData.handleArray[i];
	strncpy(pHnd->filename,filename,DXP_BUILDOPTION_FILENAMELENGTH_MAX);
	pHnd->used = 1;
	pHnd->onmemory = 0;
	pHnd->pos = 0;
	if(dxpSceIoReopen(pHnd) < 0)
	{
		pHnd->used = 0;
		ret = 0;
	}
	else
	{
		SceIoStat stat;
		sceIoGetstat(pHnd->filename,&stat);
		pHnd->size = stat.st_size;
		ret = i + 1;
	}
	FCRITICALSECTION_UNLOCK(i + 1);
	return ret;
}
