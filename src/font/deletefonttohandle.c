#include "../font.h"

int DeleteFontToHandle(int handle)
{
	if(handle <= 0 || handle >= DXP_BUILDOPTION_FONTHANDLE_MAX)return -1;//0番は必ず使うので消さない
	if(!dxpFontArray[handle].used)return -1;
	dxpFontReleaseHandle(handle);
	return 0;
}
