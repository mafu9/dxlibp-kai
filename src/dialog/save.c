#include "../dxlibp.h"
#include "../general.h"
#include "../cpcnv/unicode.h"
#include "../graphics.h"

#include <psputility.h>
#include <pspgu.h>
#include <psptypes.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define DXP_SAVE_DATANAME "DATA.BIN"

static PspUtilitySavedataFileData dxp_save_resource[4] = {
	{ NULL, 0, 0, 0 },
	{ NULL, 0, 0, 0 },
	{ NULL, 0, 0, 0 },
	{ NULL, 0, 0, 0 }
};

static PspUtilitySavedataListSaveNewData newData;

int SetSaveResource(int type, void * buf, unsigned int size)
{
	if ( type < 0 || type > 3 ) return -1;
	if ( !buf ) size = 0;

	dxp_save_resource[type].buf = buf;
	dxp_save_resource[type].bufSize = size;
	dxp_save_resource[type].size = size;

	return 0;
}

static int dxpSaveDialogInit(
	/* 格納する構造体 */
	SceUtilitySavedataParam * savedata,
	/* セーブモード */
	int mode,
	/* セーブするデータのバッファアドレス */
	void * buf,
	/* セーブするデータのバッファサイズ */
	unsigned int size,
	/* ms0:/PSP/SAVEDATA/GameName/って感じ */
	const char * GameName,
	/* AUTO SAVE時にセーブする場所 */
	const char * SaveName,
	/* セーブリスト */
	char (*nameMultiple)[20],
	/* ゲームのタイトル */
	const char * GameTitle,
	/* セーブデータのタイトル */
	const char * SaveTitle,
	/* セーブデータの詳細 */
	const char * SaveDetail
	)
{
	memset(savedata, 0, sizeof(SceUtilitySavedataParam));
	savedata->base.size = sizeof(SceUtilitySavedataParam);
	
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &savedata->base.language);
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN, &savedata->base.buttonSwap);

	savedata->base.graphicsThread = 0x11;
	savedata->base.accessThread = 0x13;
	savedata->base.fontThread = 0x12;
	savedata->base.soundThread = 0x10;

	savedata->mode = (PspUtilitySavedataMode)mode;
	savedata->overwrite = 1;

	//最近セーブしたファイルにフォーカス
	savedata->focus = PSP_UTILITY_SAVEDATA_FOCUS_LATEST;
	
#if _PSP_FW_VERSION >= 200
	const char key[] = "ThisIsASampleKey";
	strncpy(savedata->key, key, 16);
#endif
	
	strncpy(savedata->gameName, GameName, 13);
	savedata->gameName[12] = 0;
	strncpy(savedata->saveName, SaveName, 20);
	savedata->saveName[19] = 0;
	
	savedata->saveNameList = nameMultiple;

	strncpy(savedata->fileName, DXP_SAVE_DATANAME, 13);
	savedata->fileName[12] = 0;

	if ( size != 0 ) {
		savedata->dataBuf = malloc(size);
		if (savedata->dataBuf == NULL) return -1;
	} else {
		savedata->dataBuf = NULL;
	}

	savedata->dataBufSize = size;
	savedata->dataSize = size;

	if ( mode == PSP_UTILITY_SAVEDATA_LISTSAVE ||
		 mode == PSP_UTILITY_SAVEDATA_AUTOSAVE ||
		 mode == PSP_UTILITY_SAVEDATA_SAVE )
	{
		//データのコピー
		memset(savedata->dataBuf, 0x0, size);
		memcpy(savedata->dataBuf, buf, size);

		
		if ( dxpGeneralData.charset == DXP_CP_UTF8 ) {
			strncpy(savedata->sfoParam.title, GameTitle, 128);
			strncpy(savedata->sfoParam.savedataTitle, SaveTitle, 128);
			strncpy(savedata->sfoParam.detail, SaveDetail, 1024);
			savedata->sfoParam.title[127] = 0;
			savedata->sfoParam.savedataTitle[127] = 0;
			savedata->sfoParam.detail[1023] = 0;
			savedata->sfoParam.parentalLevel = 1;
		} else if ( dxpGeneralData.charset == DXP_CP_SJIS ) {
			//128byte
			char utf8[1024];

			sjis_to_utf8((void*)utf8, (void*)GameTitle);
			utf8[127] = '\0';
			strncpy(savedata->sfoParam.title, utf8, 128);
		
			sjis_to_utf8((void*)utf8, (void*)SaveTitle);
			utf8[127] = '\0';
			strncpy(savedata->sfoParam.savedataTitle, utf8, 128);

			//1024byte
			sjis_to_utf8((void*)utf8, (void*)SaveDetail);
			utf8[1023] = '\0';
			strncpy(savedata->sfoParam.detail, utf8, 1024);
			savedata->sfoParam.parentalLevel = 1;
		}
	
		//背景
		savedata->pic1FileData.buf = dxp_save_resource[DXP_SAVE_RESOURCE_PIC1].buf;
		savedata->pic1FileData.bufSize = dxp_save_resource[DXP_SAVE_RESOURCE_PIC1].size;
		savedata->pic1FileData.size = dxp_save_resource[DXP_SAVE_RESOURCE_PIC1].size;
		
		//アイコン
		savedata->icon0FileData.buf = dxp_save_resource[DXP_SAVE_RESOURCE_ICON0].buf;
		savedata->icon0FileData.bufSize = dxp_save_resource[DXP_SAVE_RESOURCE_ICON0].size;
		savedata->icon0FileData.size = dxp_save_resource[DXP_SAVE_RESOURCE_ICON0].size;

		//説明
		savedata->icon1FileData.buf = dxp_save_resource[DXP_SAVE_RESOURCE_ICON1].buf;
		savedata->icon1FileData.bufSize = dxp_save_resource[DXP_SAVE_RESOURCE_ICON1].size;
		savedata->icon1FileData.size = dxp_save_resource[DXP_SAVE_RESOURCE_ICON1].size;

		//音楽
		savedata->snd0FileData.buf = dxp_save_resource[DXP_SAVE_RESOURCE_SND0].buf;
		savedata->snd0FileData.bufSize = dxp_save_resource[DXP_SAVE_RESOURCE_SND0].size;
		savedata->snd0FileData.size = dxp_save_resource[DXP_SAVE_RESOURCE_SND0].size;

		const char* new_title = "新規作成";
		char new_title_utf8[20];

		sjis_to_utf8((void*)new_title_utf8, (const void*)new_title);
		new_title_utf8[19] = '\0';
		newData.title = new_title_utf8;
		savedata->newData = &newData;
	
		//空のファイルが最初
		savedata->focus = PSP_UTILITY_SAVEDATA_FOCUS_FIRSTEMPTY;
	}

	return 0;
}
	
int ShowSaveDialog(
	/* セーブモード */
	int mode,
	/* セーブデータのバッファ */
	void * buf,
	/* bufのサイズ */
	unsigned int size,
	/* ms0:/PSP/SAVEDATA/GameName */
	const char * GameName,
	/* ゲームのタイトル、セーブデータのタイトル、セーブデータの詳細 */
	const char * GameTitle, const char * SaveTitle, const char * SaveDetail )
{
	if ( mode < 0 || mode > 6 ) return -1;

	//オートセーブモードでは0000にセーブする
	char NameList[][20] = {
		 "0000",
		 "0001",
		 "0002",
		 "0003",
		 "0004",
		 "0005",
		 "0006",
		 "0007",
		 "0008",
		 "0009",
		 ""
	};

	SceUtilitySavedataParam dialog;
	if ( dxpSaveDialogInit(&dialog, mode, buf, size, GameName, "0000", NameList, GameTitle, SaveTitle, SaveDetail) < 0 )
		return -1;

	if ( sceUtilitySavedataInitStart(&dialog) != 0 ) return -1;
	int done = 0;

	while( ProcessMessage() != -1 && !done )
	{
		if ( mode != PSP_UTILITY_SAVEDATA_AUTOSAVE ) {
			ClearDrawScreen();
			if ( dxpDialogDrawScene != NULL ) dxpDialogDrawScene();
		}
		GUFINISH
		switch( sceUtilitySavedataGetStatus() ) {
		case PSP_UTILITY_DIALOG_INIT:
			break;
		case PSP_UTILITY_DIALOG_VISIBLE:
			sceUtilitySavedataUpdate(1);
			break;
		case PSP_UTILITY_DIALOG_QUIT:
			sceUtilitySavedataShutdownStart();
			break;
		case PSP_UTILITY_DIALOG_FINISHED:
			done = 1;
			break;
		case PSP_UTILITY_DIALOG_NONE:
			done = 1;
		default:
			break;
		}

		if ( mode != PSP_UTILITY_SAVEDATA_AUTOSAVE ) ScreenFlip();
	}
	
	memset(dxp_save_resource, 0, sizeof(PspUtilitySavedataFileData) * 4);

	int ret;

	if ( mode == DXP_SAVE_MODE_LOAD ) {
		//正常に読み込んだ
		//if(dialog.unknown1 != 0)
		//if(dialog.unknown1 == 1021)
		if ( dialog.base.result == 0 ) {
			memcpy(buf, dialog.dataBuf, size);
			ret = 0;
		}
		//キャンセルされた
		else if ( dialog.base.result == 1 ) {
			memset(buf, 0x0, size);
			ret = -2;
		}
		//セーブデータが存在しなかった
		//else if(dialog.base.result == -2146368761)
		else {
			memset(buf, 0x0, size);
			ret = -3;
		}
	} else {
		ret = 0;
	}
	free(dialog.dataBuf);
	return ret;
}