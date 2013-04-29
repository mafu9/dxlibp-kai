#include "../graphics.h"

int SetDrawAreaFull(void)
{
	const int x1 = 0, y1 = 0, x2 = 480, y2 = 272;
	GUINITCHECK;
	GUSTART;
	sceGuScissor(x1,y1,x2,y2);
	dxpGraphicsData.intrafont_scissor[0] = x1;
	dxpGraphicsData.intrafont_scissor[1] = y1;
	dxpGraphicsData.intrafont_scissor[2] = x2;
	dxpGraphicsData.intrafont_scissor[3] = y2;
	return 0;
}
