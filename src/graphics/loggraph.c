#include "../graphics.h"

int LogGraph(int gh)
{
	if ( gh < 0 ) return -1;
	
	GUINITCHECK;
	DXPGRAPHICSHANDLE* gptr;
	GHANDLE2GPTR(gptr,gh);
	
	char *psmstr[] = {
		"DXP_FMT_5650",
		"DXP_FMT_5551",
		"DXP_FMT_4444",
		"DXP_FMT_8888",
		"DXP_FMT_T4",
		"DXP_FMT_T8",
	};
	
	AppLogAdd("\r\n*** Graphics Information (GH = %d) *** \r\n\r", gh);
	AppLogAdd("PSM    : %s\r", psmstr[gptr->tex->psm]);
	AppLogAdd("WIDTH  : %dpx\r", gptr->tex->width);
	AppLogAdd("HEIGHT : %dpx\r", gptr->tex->height);
	AppLogAdd("PITCH  : %dpx\r", gptr->tex->pitch);
	AppLogAdd("UMAX   : %dpx\r", gptr->tex->umax);
	AppLogAdd("VMAX   : %dpx\r", gptr->tex->vmax);
	AppLogAdd("DATA(DDR)  : 0x%X\r", (u32)gptr->tex->texdata);
	AppLogAdd("DATA(VRAM) : 0x%X\r", (u32)gptr->tex->texvram);
	AppLogAdd("COLORKEY   : 0x%X\r", gptr->tex->colorkey);
	AppLogAdd("\r");
	AppLogAdd("[POW2]    -> %s\r", gptr->tex->size2_nflag ? "Yes" : "No");
	AppLogAdd("[SWIZZLE] -> %s\r", gptr->tex->swizzledflag ? "Yes" : "No");
	AppLogAdd("[ALPHA]   -> %s\r", gptr->tex->alphabit ? "Yes" : "No");
	AppLogAdd("[RELOAD]  -> %s\r", gptr->tex->reloadflag ? "Yes" : "No");	
	AppLogAdd("\r\n--- texinfo ---\r\n\r");
	AppLogAdd("U0 - %dpx\r", gptr->u0);
	AppLogAdd("U1 - %dpx\r", gptr->u1);
	AppLogAdd("V0 - %dpx\r", gptr->v0);
	AppLogAdd("V1 - %dpx\r", gptr->v1);	
	AppLogAdd("\r\n\r");
	
	return 0;
}
