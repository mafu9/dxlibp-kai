#include "../graphics.h"
int ChangeJpegSetting(int quality, int pro)
{
	if ( quality < 1 || 100 < quality ) return -1;
	dxpjpegsetting.jquality = quality;
	dxpjpegsetting.progression = ( !pro ? 0 : 1 );
	return 0;
}