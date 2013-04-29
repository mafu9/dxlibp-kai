#include "../general.h"
#include "../font.h"

int SetCodepointToHandle(int codepoint,int handle)
{
	if(handle < 0 || handle >= DXP_BUILDOPTION_FONTHANDLE_MAX)return -1;
	if(!dxpFontData.init)dxpFontInit();
	DXPFONTHANDLE *pHnd;
	pHnd = &dxpFontArray[handle];
	if(!pHnd->used)return -1;
	
#ifndef DXP_BUILDOPTION_6XX_SJISENCODING
	switch(codepoint)
	{
	case DXP_CP_UTF8:
		intraFontSetEncoding(pHnd->pif,INTRAFONT_STRING_UTF8);
		break;
	case DXP_CP_SJIS:
		intraFontSetEncoding(pHnd->pif,INTRAFONT_STRING_SJIS);
		break;
	default:
		return -1;
	}
#endif
	dxpGeneralData.charset = codepoint;
	return 0;
}

int SetCodepoint(int codepoint)
{
	return SetCodepointToHandle(codepoint, 0);
}