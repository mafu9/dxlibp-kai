#include "dxlibp.h"
#include <malloc.h>
#include <stdio.h>

//macros ---

//stream
#define STREAM_SEEKTYPE_SET							(PSP_SEEK_SET)
#define STREAM_SEEKTYPE_END							(PSP_SEEK_END)
#define STREAM_SEEKTYPE_CUR							(PSP_SEEK_CUR)

//structures ----

typedef struct DXPFILEIOHANDLE__
{
	unsigned used : 1;
	unsigned onmemory : 1;

	char filename[DXP_BUILDOPTION_FILENAMELENGTH_MAX];

	u32 pos;
	u32 size;

	union
	{
		SceUID fd;
		const void *dat;
	};
}DXPFILEIOHANDLE;

typedef struct DXPFILEIODATA__
{
	unsigned init : 1;
	unsigned sleep : 1;
	DXPFILEIOHANDLE handleArray[DXP_BUILDOPTION_FILEHANDLE_MAX];
	SceUID eventFlags[(DXP_BUILDOPTION_FILEHANDLE_MAX + 31) / 32];
}DXPFILEIODATA;
//variables ----

extern DXPFILEIODATA dxpFileioData;

//local functions ----



int dxpFileioInit(void);
int dxpFileioReopen(DXPFILEIOHANDLE *pHnd);
int dxpFileioOpenOnMemory(const void *buffer, u32 size);


#define FCRITICALSECTION_LOCK(HANDLE) sceKernelWaitEventFlag(dxpFileioData.eventFlags[(HANDLE) / 32 - 1], 1 << ((HANDLE) % 32), PSP_EVENT_WAITAND|PSP_EVENT_WAITCLEAR, NULL, NULL)
#define FCRITICALSECTION_UNLOCK(HANDLE) sceKernelSetEventFlag(dxpFileioData.eventFlags[(HANDLE) / 32 - 1], 1 << ((HANDLE) % 32))
#define FHANDLE2PTR(PTR,HANDLE)												\
{																			\
	if(HANDLE <= 0 || HANDLE > DXP_BUILDOPTION_FILEHANDLE_MAX)return -1;	\
	PTR = dxpFileioData.handleArray + (HANDLE) - 1;							\
	FCRITICALSECTION_LOCK(HANDLE);											\
	if(!(PTR)->used)														\
	{																		\
		FCRITICALSECTION_UNLOCK(HANDLE);									\
		return -1;															\
	}																		\
}

