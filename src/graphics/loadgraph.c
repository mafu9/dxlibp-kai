#include "../graphics.h"
#include "../fileio.h"

int LoadGraph(const char *FileName)
{
	int fileHandle, graphHandle;
	GUINITCHECK;
	if(!FileName)return -1;
	fileHandle = FileRead_open(FileName, 0);
	if(fileHandle == 0)return -1;
	graphHandle = dxpGraphLoadFromFileHandle(fileHandle);
	FileRead_close(fileHandle);
	return graphHandle;
}

