#include "../mutex.h"
#include "../memory.h"

unsigned int GetMemoryFreeSize(int fast)
{
	const unsigned int minSize = fast ? 1024 : 2;
	unsigned int bufSize = 20 * 1024 * 1024, freeSize = 0;
	DXPMEMORYLIST *top = NULL, *curr = NULL, *prev = NULL, *next = NULL;

	LockMutex(dxpMemoryData.mutexHandle);

	while(1)
	{
		DXPMEMORYLIST *curr = (DXPMEMORYLIST*)malloc(sizeof(DXPMEMORYLIST));
		if(curr == NULL)break;
		curr->buf = malloc(bufSize);
		if(curr->buf == NULL)
		{
			free(curr);
			freeSize += sizeof(DXPMEMORYLIST);
			if(bufSize <= minSize) break;
			bufSize /= 2;
			continue;
		}
		freeSize += bufSize;
		if(prev == NULL)top = curr;
		else prev->next = curr;
		curr->next = NULL;
		prev = curr;
	}
	for(curr = top; curr != NULL; curr = next)
	{
		next = curr->next;
		free(curr->buf);
		free(curr);
	}

	UnlockMutex(dxpMemoryData.mutexHandle);

	return freeSize;
}

