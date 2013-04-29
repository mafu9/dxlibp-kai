#include "../sound.h"
#include "../fileio.h"
#include <string.h>
#include <pspaudio.h>
#include "../general.h"
#include "../safealloc.h"

int dxpSoundThreadFunc_file(SceSize size,void* argp)
{
	int channel = -1;
	u32 *pcmBuf[2] = {NULL,NULL};
	u32 pcmBufSize[2] = {0,0};
	u8 pcm = 1;
	DXPSOUNDHANDLE *pHnd = dxpSoundArray + *(int*)argp;
	while(dxpSoundData.init)
	{
		sceKernelDelayThread(1000);
		pcm ^= 1;
		if(dxpGeneralData.exit_called)break;
		if(dxpGeneralData.homebutton_pushed)break;
		if(pHnd->cmd == DXP_SOUNDCMD_EXIT)break;
		switch(pHnd->cmd)
		{
		case DXP_SOUNDCMD_NONE:
			break;
		case DXP_SOUNDCMD_PLAY:
			pHnd->playing = 1;
			pHnd->cmd = DXP_SOUNDCMD_NONE;
			break;
		case DXP_SOUNDCMD_STOP:
			pHnd->playing = 0;
			pHnd->cmd = DXP_SOUNDCMD_NONE;
			break;
		}

		if ( pHnd->playing ) {
			//チャンネルの取得
			if ( channel < 0 ) channel = dxpSoundAudioChReserve(&pHnd->avContext);
			if ( channel < 0 ) continue;
			
			//バッファの確保
			if ( pcmBufSize[pcm] < dxpSoundCalcBufferSize(&pHnd->avContext, pHnd->avContext.outSampleNum) ) {
				dxpSafeFree(pcmBuf[pcm]);
				pcmBuf[pcm] = dxpSafeAlloc(dxpSoundCalcBufferSize(&pHnd->avContext, pHnd->avContext.outSampleNum));
				if ( !pcmBuf[pcm] ) {
					pcmBufSize[pcm] = 0;
					pHnd->playing = 0;
					continue;
				}
				pcmBufSize[pcm] = dxpSoundCalcBufferSize(&pHnd->avContext, pHnd->avContext.outSampleNum);
			}
			pHnd->avContext.pcmOut = pcmBuf[pcm];

			if ( pHnd->file.gotoPos >= 0 ) {
				dxpSoundCodecSeek(pHnd,pHnd->file.gotoPos);
				dxpSoundCodecDecode(pHnd);
				pHnd->file.gotoPos = -1;
			}

			//デコード
			if ( dxpSoundCodecDecode(pHnd) < 0 ) {
				if ( pHnd->file.loop ) {
					dxpSoundCodecSeek(pHnd, pHnd->loopResumePos);
					continue;
				}
				dxpSoundCodecSeek(pHnd,0);
				pHnd->playing = 0;
				continue;
			}

			while ( sceAudioGetChannelRestLen(channel) > 0 ) sceKernelDelayThread(1000);
			sceAudioSetChannelDataLen(channel, PSP_AUDIO_SAMPLE_ALIGN(pHnd->avContext.outSampleNum));
			sceAudioOutputPanned(
				channel,
				PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
				PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan / 10000.0f : 1.0f) * pHnd->volume / 255.0f,
				pcmBuf[pcm]
			);
		}else
		{
			if(channel >= 0)
			{
				sceAudioChRelease(channel);
				channel = -1;
			}
		}
	}
	dxpSafeFree(pcmBuf[0]);
	dxpSafeFree(pcmBuf[1]);
	if(channel >= 0)sceAudioChRelease(channel);
	pHnd->file.threadId = -1;
	pHnd->cmd = DXP_SOUNDCMD_NONE;
	pHnd->playing = 0;
	sceKernelExitDeleteThread(0);
	return 0;
}

int memnopress_handle[PSP_AUDIO_CHANNEL_MAX];
int memnopress_pos[PSP_AUDIO_CHANNEL_MAX];
int memnopress_playtype[PSP_AUDIO_CHANNEL_MAX];
int memnopress_channel[PSP_AUDIO_CHANNEL_MAX];

int dxpSoundThreadFunc_memnopress(SceSize len,void* ptr)
{
	int i,j;
	for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i) {
		memnopress_handle[i] = -1;
		memnopress_channel[i] = -1;
	}

	while(dxpSoundData.init)
	{
		sceKernelDelayThread(1000);
		if(dxpGeneralData.exit_called)break;
		if(dxpGeneralData.homebutton_pushed)break;

		//コマンド受け取り
		for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
		{
			if(!dxpSoundArray[i].used)continue;
			if(dxpSoundArray[i].soundDataType != DX_SOUNDDATATYPE_MEMNOPRESS)continue;
			switch(dxpSoundArray[i].cmd)
			{
			case DXP_SOUNDCMD_NONE:
				continue;
			case DXP_SOUNDCMD_PLAY:
				for( j = 0; j < PSP_AUDIO_CHANNEL_MAX; j++ ) {
					if( memnopress_handle[j] < 0 ) {
						memnopress_channel[j] = dxpSoundAudioChReserve(&dxpSoundArray[i].avContext);
						if ( memnopress_channel[j] < 0 ) break;
						memnopress_handle[j] = i;
						memnopress_pos[j] = 0;
						memnopress_playtype[j] = dxpSoundArray[i].memnopress.cmdplaytype;
						break;
					}
				}
				dxpSoundArray[i].cmd = DXP_SOUNDCMD_NONE;
				break;
			case DXP_SOUNDCMD_STOP:
			case DXP_SOUNDCMD_EXIT:
			default:
				for(j = 0;j < PSP_AUDIO_CHANNEL_MAX;++j) {
					if(memnopress_handle[j] == i) {
						if(memnopress_channel[j] >= 0)sceAudioChRelease(memnopress_channel[j]);
						memnopress_channel[j] = -1;
						--dxpSoundArray[memnopress_handle[j]].playing;
						memnopress_handle[j] = -1;
					}
				}
				dxpSoundArray[i].cmd = DXP_SOUNDCMD_NONE;
				break;			
			}
		}

		//ループ等制御
		for( i = 0; i < PSP_AUDIO_CHANNEL_MAX; ++i) {
			if( memnopress_handle[i] < 0 ) continue;

			if ( memnopress_pos[i] >= dxpSoundArray[memnopress_handle[i]].memnopress.length ) {
				if ( memnopress_playtype[i] == DX_PLAYTYPE_LOOP ) {
					memnopress_pos[i] = dxpSoundArray[memnopress_handle[i]].loopResumePos;
				} else {
					if ( memnopress_channel[i] >= 0 ) sceAudioChRelease(memnopress_channel[i]);
					memnopress_channel[i] = -1;
					memnopress_handle[i] = -1;
					--dxpSoundArray[memnopress_handle[i]].playing;
					continue;
				}
			}

		//再生バッファ監視
		//再生
			if ( sceAudioGetChannelRestLen(memnopress_channel[i]) <= 0 ) {
				//sceAudioSetChannelDataLen(channel, PSP_AUDIO_SAMPLE_ALIGN(dxpSoundArray[memnopress_handle[i]].avContext.outSampleNum));
				sceAudioOutputPanned(
					memnopress_channel[i],
					PSP_AUDIO_VOLUME_MAX * (dxpSoundArray[memnopress_handle[i]].pan > 0 ? 1.0f - dxpSoundArray[memnopress_handle[i]].pan / 10000.0f : 1.0f) * dxpSoundArray[memnopress_handle[i]].volume / 255.0f,
					PSP_AUDIO_VOLUME_MAX * (dxpSoundArray[memnopress_handle[i]].pan < 0 ? 1.0f + dxpSoundArray[memnopress_handle[i]].pan / 10000.0f : 1.0f) * dxpSoundArray[memnopress_handle[i]].volume / 255.0f,
					dxpSoundArray[memnopress_handle[i]].memnopress.pcmBuf + memnopress_pos[i]
				);
				//memnopress_pos[i] += dxpSoundGetNextSampleNum(&dxpSoundArray[memnopress_handle[i]].avContext);
				memnopress_pos[i] += dxpSoundArray[memnopress_handle[i]].avContext.outSampleNum;
			}
		}
	}
	for(i = 0;i < PSP_AUDIO_CHANNEL_MAX;++i) {
		if(memnopress_channel[i] >= 0)sceAudioChRelease(memnopress_channel[i]);
		if(memnopress_handle[i] >= 0) {
			dxpSoundArray[memnopress_handle[i]].playing = 0;
		}
	}
	sceKernelExitDeleteThread(0);
	return 0;
}
