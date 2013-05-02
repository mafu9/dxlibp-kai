#include <psputility.h>
#include <malloc.h>
#include <string.h>
#include "../dxlibp.h"
#include "../general.h"
#include "../graphics.h"
#include "../cpcnv/unicode.h"


DialogCallback dxpDialogDrawScene = NULL;

void dxpDialogParamInit(pspUtilityMsgDialogParams *params)
{
	memset(params,0,sizeof(pspUtilityMsgDialogParams));
	params->base.size = sizeof(pspUtilityMsgDialogParams);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params->base.language);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &params->base.buttonSwap);
	params->base.graphicsThread = 0x11;
	params->base.accessThread = 0x13;
	params->base.fontThread = 0x12;
	params->base.soundThread = 0x10;
}

int dxpShowDialog(pspUtilityMsgDialogParams *params)
{
	sceUtilityMsgDialogInitStart(params);

	int done = 0;
	while(ProcessMessage() != -1 && !done)
	{
		ClearDrawScreen();
		if ( dxpDialogDrawScene != NULL ) dxpDialogDrawScene();
		GUFINISH
		switch(sceUtilityMsgDialogGetStatus())
		{
			case PSP_UTILITY_DIALOG_INIT:
				break;

			case PSP_UTILITY_DIALOG_VISIBLE:
				sceUtilityMsgDialogUpdate(1);
				break;

			case PSP_UTILITY_DIALOG_QUIT:
				sceUtilityMsgDialogShutdownStart();
				break;

			case PSP_UTILITY_DIALOG_FINISHED:
				done = 1;
				break;

			case PSP_UTILITY_DIALOG_NONE:
				done = 1;
				break;

			default:
				break;
		}
		ScreenFlip();
	}

	return params->buttonPressed;
}

int ShowTextDialog(const char *text,int options)
{
	options |= PSP_UTILITY_MSGDIALOG_OPTION_TEXT;

	pspUtilityMsgDialogParams params;
	dxpDialogParamInit(&params);
	params.mode = PSP_UTILITY_MSGDIALOG_MODE_TEXT;
	params.options = options;

	if ( dxpGeneralData.charset == DXP_CP_UTF8 ) {
		strncpy( params.message, text, 512 );
	} else if ( dxpGeneralData.charset == DXP_CP_SJIS ) {
		char utf8[512];
		sjis_to_utf8((void*)utf8, (void*)text);
		utf8[511] = '\0';
		strcpy( params.message, utf8 );
	} else {
		strncpy( params.message, text, 512 );
	}
	params.message[511] = 0;

	return dxpShowDialog(&params);
}

int ShowErrorDialog(unsigned int errorvalue)
{
	pspUtilityMsgDialogParams params;
	dxpDialogParamInit(&params);
	params.mode = PSP_UTILITY_MSGDIALOG_MODE_ERROR;
	params.options = PSP_UTILITY_MSGDIALOG_OPTION_ERROR;
	params.errorValue = errorvalue;

	return dxpShowDialog(&params);
}

int SetDialogDrawScene(DialogCallback func)
{
	dxpDialogDrawScene = func;
	return 0;
}