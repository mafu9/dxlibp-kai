#include "../memory.h"
#include <stdio.h>
#include <stdlib.h>

void *__real_malloc(size_t size);
void *__real_realloc(void *ptr, size_t size);
void *__real_calloc(size_t n, size_t size);
void *__real_memalign(size_t align, size_t size);
void __real_free(void *ptr);

void *__wrap_malloc(size_t size);
void *__wrap_realloc(void *ptr, size_t size);
void *__wrap_calloc(size_t n, size_t size);
void *__wrap_memalign(size_t align, size_t size);
void __wrap_free(void *ptr);

DXPMEMORYDATA dxpMemoryData = {0, 1, -1, -1, 0, 0};

int dxpMemoryInit(void)
{
	if(dxpMemoryData.init)return 0;
	dxpMemoryData.mutex = sceKernelCreateSema("dxpmemorysema", 0, 1, 1, NULL);
	if(dxpMemoryData.mutex < 0)return -1;
	dxpMemoryData.init = 1;
	dxpMemoryData.memAssert = 1;
	dxpMemoryData.mutexOwnerThid = -1;
	dxpMemoryData.mutexCount = 0;
	dxpMemoryData.totalMemSize = GetMemoryFreeSize(FALSE);
	return 0;
}

int dxpMemoryEnd(void)
{
	if(!dxpMemoryData.init)return 0;
	sceKernelDeleteSema(dxpMemoryData.mutex);
	dxpMemoryData.init = 0;
	return 0;
}

void dxpMemoryLock(void)
{
	const SceUID thid = sceKernelGetThreadId();
	if(thid != dxpMemoryData.mutexOwnerThid)
	{
		sceKernelWaitSema(dxpMemoryData.mutex, 1, NULL);
		dxpMemoryData.mutexOwnerThid = thid;
	}
	else
	{
		dxpMemoryData.mutexCount++;
	}
}

void dxpMemoryUnlock(void)
{
	if(dxpMemoryData.mutexCount > 0)
	{
		dxpMemoryData.mutexCount--;
	}
	else
	{
		dxpMemoryData.mutexOwnerThid = -1;
		sceKernelSignalSema(dxpMemoryData.mutex, 1);
	}
}

static int dxpMemoryIsSingleLocked(void)
{
	return (dxpMemoryData.mutexCount == 0);
}

static void dxpMemoryError(void)
{
}

void* __wrap_malloc(size_t size)
{
	void *ptr;
	dxpMemoryLock();
	ptr = __real_malloc(size);
	if(ptr == NULL && size != 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "malloc failed: %ubytes\n", size);
		dxpMemoryError();
	}
	dxpMemoryUnlock();
	return ptr;
}

void* __wrap_realloc(void *ptr, size_t size)
{
	void *newPtr;
	dxpMemoryLock();
	newPtr = __real_realloc(ptr, size);
	if(newPtr == NULL && size != 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "realloc failed: %p %ubytes\n", ptr, size);
		dxpMemoryError();
	}
	dxpMemoryUnlock();
	return newPtr;
}

void *__wrap_calloc(size_t n, size_t size)
{
	void *ptr;
	dxpMemoryLock();
	ptr = __real_calloc(n, size);
	if(ptr == NULL && n != 0 && size != 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "calloc failed: %ublocks %ubytes\n", n, size);
		dxpMemoryError();
	}
	dxpMemoryUnlock();
	return ptr;
}

void* __wrap_memalign(size_t align, size_t size)
{
	void *ptr;
	dxpMemoryLock();
	ptr = __real_memalign(align, size);
	if(ptr == NULL && size > 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "memalign failed: %ubytes(aligned %ubytes)\n", size, align);
		dxpMemoryError();
	}
	dxpMemoryUnlock();
	return ptr;
}

void __wrap_free(void *ptr)
{
	dxpMemoryLock();
	__real_free(ptr);
	dxpMemoryUnlock();
}
