#include "../sound.h"
#include "../fileio.h"
#include <string.h>
#include <pspaudio.h>
#include "../general.h"

DXPSOUNDHANDLE dxpSoundArray[DXP_BUILDOPTION_SOUNDHANDLE_MAX];
DXPSOUNDDATA dxpSoundData = {0,DX_SOUNDDATATYPE_MEMNOPRESS};

int dxpSoundAudioChReserve(DXPAVCONTEXT *av)
{
	int format = ( av->channels == 2 ) ? PSP_AUDIO_FORMAT_STEREO : PSP_AUDIO_FORMAT_MONO;
	int samplecount = PSP_AUDIO_SAMPLE_ALIGN(av->outSampleNum);
	return sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, samplecount, format);
}

int dxpSoundCalcBufferSize(DXPAVCONTEXT *av, int sample)
{
	return sample * av->sampleSize * av->channels;
}

int dxpSoundGetNextSampleNum(DXPAVCONTEXT *av)
{
	int ret = av->outSampleNum;
	if ( av->nextPos + av->outSampleNum > av->totalSampleNum )
		ret = av->totalSampleNum - av->nextPos;
	return ret;
}

int dxpSoundInit()
{
	if(dxpSoundData.init)return 0;
	int ret;
	ret = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if(ret < 0)return -1;
	memset(dxpSoundArray,0,sizeof(dxpSoundArray));
	dxpSoundData.init = 1;
	sceKernelStartThread(sceKernelCreateThread("dxp mnp sound thread",dxpSoundThreadFunc_memnopress,0x11,0x4000,PSP_THREAD_ATTR_USER,0),0,0);
	return 0;
}

int dxpSoundTerm()
{
	if(!dxpSoundData.init)return 0;
	//全部に対してStopをかける
	int i;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
		DeleteSoundMem(i);
	sceUtilityUnloadAvModule(PSP_MODULE_AV_AVCODEC);
	dxpSoundData.init = 0;
	return 0;
}

int dxpSoundReserveHandle()
{
	if(!dxpSoundData.init)return -1;
	int i;
	DXPSOUNDHANDLE *pHnd;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
	{
		if(dxpSoundArray[i].used)continue;
		pHnd = dxpSoundArray + i;
		memset(pHnd,0,sizeof(DXPSOUNDHANDLE));
		pHnd->volume = 255;
		pHnd->used = 1;
		return i;
	}
	return -1;
}

int dxpSoundReleaseHandle(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle, pHnd);
	pHnd->used = 0;
	return 0;
}

int GetSoundInfo(int handle, DXP_SOUND_DATA *dest)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle, pHnd);
	dest->data = (void *)pHnd->memnopress.pcmBuf;
	dest->sample_len = pHnd->avContext.totalSampleNum;
	dest->sample_rate = pHnd->avContext.sampleRate;
	dest->sample_size = pHnd->avContext.sampleSize;
	dest->channels = pHnd->avContext.channels;
	return 0;
}

int LoadSoundMem(const char *filename)
{
	if ( !dxpSoundData.init ) return -1;

	int fileSize = FileRead_size(filename);
	if ( fileSize <= 0 ) return -1;

	int fileHandle = FileRead_open(filename, 0);
	if ( fileHandle == 0 ) return -1;

	int handle = dxpSoundReserveHandle();
	if ( handle < 0 ) {
		FileRead_close(fileHandle);
		return -1;
	}

	DXPSOUNDHANDLE *pHnd = dxpSoundArray + handle;
	pHnd->soundDataType = dxpSoundData.createSoundDataType;

	if(pHnd->soundDataType == DX_SOUNDDATATYPE_MEMPRESS)
	{
		pHnd->file.buffer = malloc(fileSize);
		if(!pHnd->file.buffer)
		{
			dxpSoundReleaseHandle(handle);
			FileRead_close(fileHandle);
			return -1;
		}
		FileRead_read(pHnd->file.buffer, fileSize, fileHandle);
		FileRead_close(fileHandle);
		fileHandle = dxpFileioOpenOnMemory(pHnd->file.buffer, fileSize);
		if(fileHandle == 0)
		{
			free(pHnd->file.buffer);
			dxpSoundReleaseHandle(handle);
			return -1;
		}
	}

	pHnd->avContext.fileHandle = fileHandle;
	pHnd->avContext.fileSize = fileSize;

	if ( dxpSoundCodecInit(pHnd) < 0 ) {
		FileRead_close(fileHandle);
		if(pHnd->soundDataType == DX_SOUNDDATATYPE_MEMPRESS)free(pHnd->file.buffer);
		dxpSoundReleaseHandle(handle);
		return -1;
	}
	pHnd->avContext.totalSampleNum = dxpSoundCodecGetSampleLength(pHnd);

	switch(pHnd->soundDataType) {
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		pHnd->memnopress.length = pHnd->avContext.totalSampleNum;
		int bufsize = dxpSoundCalcBufferSize(&pHnd->avContext, pHnd->memnopress.length);
		bufsize = ((bufsize / pHnd->avContext.outSampleNum) + 1) * pHnd->avContext.outSampleNum;
		pHnd->memnopress.pcmBuf = (u8*)memalign(64, bufsize);
		if ( !pHnd->memnopress.pcmBuf ) {
			dxpSoundCodecEnd(pHnd);
			FileRead_close(fileHandle);
			dxpSoundReleaseHandle(handle);
			return -1;
		}
		memset(pHnd->memnopress.pcmBuf, 0, bufsize);
		pHnd->avContext.nextPos = 0;
		if ( pHnd->avContext.format == DXP_SOUNDFMT_WAV ) {
			pHnd->avContext.pcmOut = pHnd->memnopress.pcmBuf;
			dxpSoundWavAllDecode(&pHnd->avContext);
		} else {
			for ( ; pHnd->avContext.nextPos < pHnd->memnopress.length; ) {
				pHnd->avContext.pcmOut = pHnd->memnopress.pcmBuf + dxpSoundCalcBufferSize(&pHnd->avContext, pHnd->avContext.nextPos);
				if ( dxpSoundCodecDecode(pHnd) < 0 ) break;
			}
		}
		dxpSoundCodecEnd(pHnd);
		FileRead_close(fileHandle);
		break;

	case DX_SOUNDDATATYPE_MEMPRESS:
	case DX_SOUNDDATATYPE_FILE:
		pHnd->file.threadId = sceKernelCreateThread("dxp sound file thread", dxpSoundThreadFunc_file, 0x11, 0x4000, PSP_THREAD_ATTR_USER, 0);
		if ( pHnd->file.threadId < 0 ) {
			dxpSoundCodecEnd(pHnd);
			FileRead_close(fileHandle);
			if(pHnd->soundDataType == DX_SOUNDDATATYPE_MEMPRESS)free(pHnd->file.buffer);
			dxpSoundReleaseHandle(handle);
			return -1;
		}

		int stat = sceKernelStartThread(pHnd->file.threadId, sizeof(handle), &handle);
		if ( stat < 0 ) {
			sceKernelDeleteThread(pHnd->file.threadId);
			dxpSoundCodecEnd(pHnd);
			FileRead_close(fileHandle);
			if(pHnd->soundDataType == DX_SOUNDDATATYPE_MEMPRESS)free(pHnd->file.buffer);
			dxpSoundReleaseHandle(handle);
			return -1;
		}
		pHnd->file.gotoPos = -1;
		break;

	default:
		dxpSoundCodecEnd(pHnd);
		FileRead_close(fileHandle);
		dxpSoundReleaseHandle(handle);
		return -1;
	}

	return handle;
}

int PlaySoundMem(int handle,int playtype,int rewindflag)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle, pHnd);
//	while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		if(playtype == DX_PLAYTYPE_NORMAL)
		{
			int channel = dxpSoundAudioChReserve(&pHnd->avContext);
			if ( channel < 0 ) return 0;
			int pos = 0;
			while ( pos < pHnd->memnopress.length ) {
				sceAudioOutputPannedBlocking(
					channel,
					PSP_AUDIO_VOLUME_MAX * (pHnd->pan > 0 ? 1.0f - pHnd->pan * 0.0001f : 1.0f) * pHnd->volume / 255.0f,
					PSP_AUDIO_VOLUME_MAX * (pHnd->pan < 0 ? 1.0f + pHnd->pan * 0.0001f : 1.0f) * pHnd->volume / 255.0f,
					pHnd->memnopress.pcmBuf + dxpSoundCalcBufferSize(&pHnd->avContext, pos)
				);
				pos += pHnd->avContext.outSampleNum;
			}
			sceAudioChRelease(channel);
		}
		else
		{
			while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
			pHnd->memnopress.cmdplaytype = playtype;
			pHnd->cmd = DXP_SOUNDCMD_PLAY;
		}
		break;

	case DX_SOUNDDATATYPE_MEMPRESS:
	case DX_SOUNDDATATYPE_FILE:
		pHnd->file.loop = playtype == DX_PLAYTYPE_LOOP ? 1 : 0;
		if(rewindflag)pHnd->file.gotoPos = 0;
		while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
		pHnd->cmd = DXP_SOUNDCMD_PLAY;
		if(playtype == DX_PLAYTYPE_NORMAL)
		{
			while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
			while(pHnd->playing > 0)sceKernelDelayThread(100);
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int StopSoundMem(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
//	while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
	pHnd->cmd = DXP_SOUNDCMD_STOP;
	return 0;
}

int DeleteSoundMem(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	while(pHnd->cmd != DXP_SOUNDCMD_NONE)sceKernelDelayThread(100);
	pHnd->cmd = DXP_SOUNDCMD_EXIT;
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		while(pHnd->playing > 0)sceKernelDelayThread(100);
		free(pHnd->memnopress.pcmBuf);
		dxpSoundReleaseHandle(handle);
		break;
	case DX_SOUNDDATATYPE_MEMPRESS:
	case DX_SOUNDDATATYPE_FILE:
		while(pHnd->playing > 0)sceKernelDelayThread(100);
		dxpSoundCodecEnd(pHnd);
		FileRead_close(pHnd->avContext.fileHandle);
		if(pHnd->soundDataType == DX_SOUNDDATATYPE_MEMPRESS)free(pHnd->file.buffer);
		dxpSoundReleaseHandle(handle);
		break;
	default:
		return -1;
	}
	return 0;
}

int CheckSoundMem(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	return pHnd->playing ? 1 : 0;
}

int InitSoundMem()
{
	if(!dxpSoundData.init)return -1;
	int i;
	for(i = 0;i < DXP_BUILDOPTION_SOUNDHANDLE_MAX;++i)
		DeleteSoundMem(i);
	return 0;
}

int SetCreateSoundDataType(int type)
{
	switch(type)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
	case DX_SOUNDDATATYPE_MEMPRESS:
	case DX_SOUNDDATATYPE_FILE:
		dxpSoundData.createSoundDataType = type;
		return 0;
	default:
		return -1;
	}
}

int SetPanSoundMem(int pan,int handle)
{
	if(pan > 10000)pan = 10000;
	if(pan < -10000)pan = -10000;
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	pHnd->pan = pan;
	return 0;
}

int ChangeVolumeSoundMem(int volume,int handle)
{
	if(volume > 255)volume = 255;
	if(volume < 0)volume = 0;
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	pHnd->volume = volume;
	return 0;
}

int SetLoopPosSoundMem(int looppos_s,int handle)
{
	if(looppos_s < 0)return -1;
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	looppos_s = pHnd->avContext.sampleRate * looppos_s;
	return SetLoopSamplePosSoundMem(looppos_s,handle);
}

int SetLoopSamplePosSoundMem(int looppos,int handle)
{
	if(looppos < 0)return -1;
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		if(looppos >= pHnd->memnopress.length)return -1;
		break;
	case DX_SOUNDDATATYPE_MEMPRESS:
	case DX_SOUNDDATATYPE_FILE:
		break;
	default:
		return -1;
	}
	pHnd->loopResumePos = looppos;
	return 0;
}

int GetSoundPlayTime(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	return pHnd->avContext.totalSampleNum / pHnd->avContext.sampleRate;
}

int GetSoundPlayedTime(int handle)
{
	DXPSOUNDHANDLE *pHnd;
	SHND2PTR(handle,pHnd);
	switch(pHnd->soundDataType)
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS:
		{
			int i;
			for ( i = 0; i < PSP_AUDIO_CHANNEL_MAX; i++ ) {
				if ( memnopress_handle[i] == handle )
					return memnopress_pos[i] / pHnd->avContext.sampleRate;
			}
			return 0;
		}
	case DX_SOUNDDATATYPE_MEMPRESS:
	case DX_SOUNDDATATYPE_FILE:
		return pHnd->avContext.nextPos / pHnd->avContext.sampleRate;
	default:
		return -1;
	}
	return 0;
}