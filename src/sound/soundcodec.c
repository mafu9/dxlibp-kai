#include "../sound.h"

int dxpSoundCodecInit(DXPSOUNDHANDLE *pHnd)
{
	int status;
	if ( !dxpSoundData.init ) return -1;

	status = dxpSoundMp3Init(&pHnd->avContext);
	if ( !(status < 0) ) return 0;

	status = dxpSoundOggInit(&pHnd->avContext);
	if ( !(status < 0) ) return 0;
	
	status = dxpSoundAt3Init(&pHnd->avContext);
	if ( !(status < 0) ) return 0;

	status = dxpSoundWavInit(&pHnd->avContext);
	if ( !(status < 0) ) return 0;

	return -1;
}

/* ‘ƒTƒ“ƒvƒ‹”‚ð•Ô‹p */
int dxpSoundCodecGetSampleLength(DXPSOUNDHANDLE *pHnd)
{
	if ( !dxpSoundData.init ) return -1;
	switch ( pHnd->avContext.format ) {
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3GetSampleLength(&pHnd->avContext);
	case DXP_SOUNDFMT_OGG:
		return dxpSoundOggGetSampleLength(&pHnd->avContext);
	case DXP_SOUNDFMT_AT3:
		return dxpSoundAt3GetSampleLength(&pHnd->avContext);
	case DXP_SOUNDFMT_WAV:
		return dxpSoundWavGetSampleLength(&pHnd->avContext);
	default:
		return -1;
	}
}

int dxpSoundCodecSeek(DXPSOUNDHANDLE *pHnd, int sample)
{
	if ( !dxpSoundData.init ) return -1;
	switch ( pHnd->avContext.format ) {
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Seek(&pHnd->avContext,sample);
	case DXP_SOUNDFMT_OGG:
		return dxpSoundOggSeek(&pHnd->avContext,sample);
	case DXP_SOUNDFMT_AT3:
		return dxpSoundAt3Seek(&pHnd->avContext,sample);
	case DXP_SOUNDFMT_WAV:
		return dxpSoundWavSeek(&pHnd->avContext,sample);
	default:
		return -1;
	}
}

int dxpSoundCodecDecode(DXPSOUNDHANDLE *pHnd)
{
	if ( !dxpSoundData.init ) return -1;
	switch ( pHnd->avContext.format ) {
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3Decode(&pHnd->avContext);
	case DXP_SOUNDFMT_OGG:
		return dxpSoundOggDecode(&pHnd->avContext);
	case DXP_SOUNDFMT_AT3:
		return dxpSoundAt3Decode(&pHnd->avContext);
	case DXP_SOUNDFMT_WAV:
		return dxpSoundWavDecode(&pHnd->avContext);
	default:
		return -1;
	}
}

int dxpSoundCodecEnd(DXPSOUNDHANDLE *pHnd)
{
	if ( !dxpSoundData.init ) return -1;
	switch ( pHnd->avContext.format ) {
	case DXP_SOUNDFMT_MP3:
		return dxpSoundMp3End(&pHnd->avContext);
	case DXP_SOUNDFMT_OGG:
		return dxpSoundOggEnd(&pHnd->avContext);
	case DXP_SOUNDFMT_AT3:
		return dxpSoundAt3End(&pHnd->avContext);
	case DXP_SOUNDFMT_WAV:
		return dxpSoundWavEnd(&pHnd->avContext);
	default:
		return -1;
	}
}
