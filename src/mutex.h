#include "dxlibp.h"

//macros ---

#define DXP_MUTEXHANDLE_SYSTEM_NUM 4 //DXP内部で使用するミューテックスの最大数
#define DXP_MUTEXHANDLE_MAX (DXP_MUTEXHANDLE_SYSTEM_NUM + DXP_BUILDOPTION_MUTEXHANDLE_MAX)
#define DXP_MUTEX_OWN_INDEX 0 //自分自身のミューテックス

//structures ----

typedef struct DXPMUTEX__
{
	SceUID semaid;
	SceUID ownerThid;
	int count;
}DXPMUTEX;

typedef struct DXPMUTEXDATA__
{
	DXPMUTEX handleArray[DXP_MUTEXHANDLE_MAX];
}DXPMUTEXDATA;

//variables ----

extern DXPMUTEXDATA dxpMutexData;

//local functions ----

int dxpMutexInit(void);
int dxpMutexLock(DXPMUTEX *pHnd, int tryFlag);
int dxpMutexUnlock(DXPMUTEX *pHnd);

#define MHANDLE2PTR(PTR,HANDLE) {if(HANDLE < 0 || HANDLE >= DXP_MUTEXHANDLE_MAX)return -1;PTR = dxpMutexData.handleArray + (HANDLE);}

