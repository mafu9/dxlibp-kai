#include "../graphics.h"
#include <math.h>

int DrawOval(int x,int y,int rx,int ry,int color,int fillflag)
{
#if 0
	int dx[32 + 1], dy[32 + 1], i;
	double s, c;
	GUINITCHECK;
	for(i = 0;i < 32;++i)
	{
		sincos(M_TWOPI * i / 32, &s, &c);
		dx[i] = x + rx * c;
		dy[i] = y + ry * s;
	}
	dx[32] = dx[0];
	dy[32] = dy[0];
	if(fillflag)
	{
		for(i = 0;i < 32;++i)
			DrawTriangle(x,y,dx[i],dy[i],dx[i + 1],dy[i + 1],color,TRUE);
	}else
	{
		for(i = 0;i < 32;++i)
			DrawLine(dx[i],dy[i],dx[i + 1],dy[i + 1],color);
	}
#else
	DXP_FVF_2D *vertex;
	double s, c;
	int start, i;
	GUINITCHECK;
	start = fillflag ? 1 : 0;
	dxpGraphicsSetup2D(color);
	vertex = (DXP_FVF_2D*)dxpGuGetMemory(sizeof(DXP_FVF_2D) * (start + 32 + 1));
	if(fillflag)
	{
		vertex[0].x = x;
		vertex[0].y = y;
		vertex[0].z = dxpGraphicsData.z_2d;
	}
	for(i = 0;i < 32;++i)
	{
		sincos(M_TWOPI * i / 32, &s, &c);
		vertex[start + i].x = x + rx * c;
		vertex[start + i].y = y + ry * s;
		vertex[start + i].z = dxpGraphicsData.z_2d;
	}
	vertex[start + 32] = vertex[start];
	sceGuDrawArray(fillflag ? GU_TRIANGLE_FAN : GU_LINE_STRIP,DXP_VTYPE_2D | GU_TRANSFORM_2D,start + 32 + 1,0,vertex);
#endif
	return 0;
}

