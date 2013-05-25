#include "../mutex.h"
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

DXPMEMORYDATA dxpMemoryData =
{
    .init = 0,
    .memAssert = 1,
    .mutexHandle = -1,
    .totalMemSize = 0
};

int dxpMemoryInit(void)
{
	if(dxpMemoryData.init)return 0;
	dxpMemoryData.mutexHandle = CreateMutex("dxp memory mutex");
	if(dxpMemoryData.mutexHandle < 0)return -1;
	dxpMemoryData.init = 1;
	dxpMemoryData.memAssert = 1;
	dxpMemoryData.totalMemSize = GetMemoryFreeSize(FALSE);
	return 0;
}

int dxpMemoryEnd(void)
{
	if(!dxpMemoryData.init)return 0;
	DeleteMutex(dxpMemoryData.mutexHandle);
	dxpMemoryData.init = 0;
	return 0;
}

static int dxpMemoryIsSingleLocked(void)
{
	return (GetMutexCount(dxpMemoryData.mutexHandle) == 0);
}

static void dxpMemoryError(void)
{
	fflush(stderr);
}

void* __wrap_malloc(size_t size)
{
	void *ptr;
	if(dxpMemoryData.mutexHandle != -1)LockMutex(dxpMemoryData.mutexHandle);
	ptr = __real_malloc(size);
	if(ptr == NULL && size != 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "malloc failed: %ubytes\n", size);
		dxpMemoryError();
	}
	if(dxpMemoryData.mutexHandle != -1)UnlockMutex(dxpMemoryData.mutexHandle);
	return ptr;
}

void* __wrap_realloc(void *ptr, size_t size)
{
	void *newPtr;
	if(dxpMemoryData.mutexHandle != -1)LockMutex(dxpMemoryData.mutexHandle);
	newPtr = __real_realloc(ptr, size);
	if(newPtr == NULL && size != 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "realloc failed: %p %ubytes\n", ptr, size);
		dxpMemoryError();
	}
	if(dxpMemoryData.mutexHandle != -1)UnlockMutex(dxpMemoryData.mutexHandle);
	return newPtr;
}

void *__wrap_calloc(size_t n, size_t size)
{
	void *ptr;
	if(dxpMemoryData.mutexHandle != -1)LockMutex(dxpMemoryData.mutexHandle);
	ptr = __real_calloc(n, size);
	if(ptr == NULL && n != 0 && size != 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "calloc failed: %ublocks %ubytes\n", n, size);
		dxpMemoryError();
	}
	if(dxpMemoryData.mutexHandle != -1)UnlockMutex(dxpMemoryData.mutexHandle);
	return ptr;
}

void* __wrap_memalign(size_t align, size_t size)
{
	void *ptr;
	if(dxpMemoryData.mutexHandle != -1)LockMutex(dxpMemoryData.mutexHandle);
	ptr = __real_memalign(align, size);
	if(ptr == NULL && size > 0 && dxpMemoryIsSingleLocked() && dxpMemoryData.memAssert)
	{
		fprintf(stderr, "memalign failed: %ubytes(aligned %ubytes)\n", size, align);
		dxpMemoryError();
	}
	if(dxpMemoryData.mutexHandle != -1)UnlockMutex(dxpMemoryData.mutexHandle);
	return ptr;
}

void __wrap_free(void *ptr)
{
	if(dxpMemoryData.mutexHandle != -1)LockMutex(dxpMemoryData.mutexHandle);
	__real_free(ptr);
	if(dxpMemoryData.mutexHandle != -1)UnlockMutex(dxpMemoryData.mutexHandle);
}
