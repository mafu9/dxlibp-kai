#include "../general.h"
#include "../graphics.h"
#include "../memory.h"
int DxLib_End()
{
	if(!dxpGeneralData.initialized)return 0;
//	InitSoundMem();
	dxpGraphicsEnd();
	dxpMemoryEnd();
	return 0;
}