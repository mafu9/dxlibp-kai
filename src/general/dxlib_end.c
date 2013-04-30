#include "../general.h"
#include "../sound.h"
#include "../graphics.h"
#include "../memory.h"
int DxLib_End()
{
	if(!dxpGeneralData.initialized)return 0;
//	InitSoundMem();
	dxpSoundTerm();
	dxpGraphicsEnd();
	dxpMemoryEnd();
	return 0;
}