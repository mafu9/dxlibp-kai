#include "../graphics.h"

int SetWaitVSyncFlag(int flag)
{
	GUINITCHECK;
	dxpGraphicsData.waitvsinc = flag ? 1 : 0;
	return 0;
}
