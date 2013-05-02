#include "../dxlibp.h"
#include "../general.h"
#include "../cpcnv.h"
#include <psputility.h>
#include <pspgu.h>
#include "../graphics.h"
#include <malloc.h>
#include <string.h>

int GetTextOSK(char *buf,int buflen,int mode,const char *title,const char *init)
{
	if(buf == NULL)return -1;

	u16 *winit,*wtitle,*wresult;
	char nulstr[] = "";
	if(init == NULL)init = nulstr;
	if(title == NULL)title = nulstr;
	winit = (u16*)malloc((strlen(init) + 1) * 2);
	wtitle = (u16*)malloc((strlen(title) + 1) * 2);
	wresult = (u16*)malloc((buflen + 1) * 2);
	if(winit == NULL || wtitle == NULL || wresult == NULL)
	{
		free(winit);
		free(wtitle);
		free(wresult);
		return -1;
	}


	dxpCpCode_toUcs2(winit,strlen(init) + 1,(const dxpChar*)init,dxpGeneralData.charset);
	dxpCpCode_toUcs2(wtitle,strlen(title) + 1,(const dxpChar*)title,dxpGeneralData.charset);

	SceUtilityOskData data;
	memset(&data, 0, sizeof(SceUtilityOskData));
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &data.language);
	data.unk_00 = 1;	//ATOK
	data.lines = 1;
	data.unk_24 = 1;
	data.inputtype = mode;
	data.desc = wtitle;
	data.intext = winit;
	data.outtextlength = buflen;
	data.outtextlimit = buflen;
	data.outtext = wresult;

	SceUtilityOskParams params;
	memset(&params,0x00,sizeof(params));
	params.base.size = sizeof(params);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,&params.base.language);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN,&params.base.buttonSwap);
	params.base.graphicsThread = 0x11;
	params.base.accessThread = 0x13;
	params.base.fontThread = 0x12;
	params.base.soundThread = 0x10;
	params.datacount = 1;
	params.data = &data;

	sceUtilityOskInitStart(&params);

	int done = 0;
	while(ProcessMessage() != -1 && !done)
	{
		ClearDrawScreen();
		if ( dxpDialogDrawScene != NULL ) dxpDialogDrawScene();
		GUFINISH
		switch(sceUtilityOskGetStatus())
		{
			case PSP_UTILITY_DIALOG_INIT:
				break;
			
			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityOskUpdate(1);
				break;
			
			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityOskShutdownStart();
				break;
			
			case PSP_UTILITY_DIALOG_NONE:
			case PSP_UTILITY_DIALOG_FINISHED:
				done = 1;
				break;
				
			default:
				break;
		}
		ScreenFlip();
	}

//	dxpCpSJIS_fromUcs2((dxpChar*)buf,buflen,wresult);
	dxpCpCode_fromUcs2((dxpChar*)buf,buflen,wresult,dxpGeneralData.charset);
	free(winit);
	free(wtitle);
	free(wresult);
	return 0;
}
