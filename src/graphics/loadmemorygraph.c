#include "../graphics.h"
#include "../fileio.h"

int LoadMemoryGraph(const void *data, unsigned int size)
{
	int fileHandle, graphHandle;
	GUINITCHECK;
	if(!data || !size)return -1;
    fileHandle = dxpFileioOpenOnMemory(data, size);
	if(fileHandle == 0)return -1;
	graphHandle = dxpGraphLoadFromFileHandle(fileHandle);
	FileRead_close(fileHandle);
	return graphHandle;
}

