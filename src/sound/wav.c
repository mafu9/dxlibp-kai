#include "../sound.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <psputility.h>
#include <pspaudiocodec.h>

#define WAV_FRAME_SIZE (1024)

/* -----------------------------------------------------------------
	初期化
 ----------------------------------------------------------------- */

int dxpSoundWavInit(DXPAVCONTEXT *av)
{
	u32 buffer[3] = { 0, 0, 0 };

	//RIFFヘッダの読み込み（buffer[1] -> ファイルサイズ - 8）
	FileRead_seek(av->fileHandle, 0, SEEK_SET);
	if ( FileRead_read( buffer, 8, av->fileHandle ) != 8 ) return -1;
	if ( buffer[0] != 0x46464952 ) return -1;

	//WAVEfmtチャンクの読み込み
	if ( FileRead_read( buffer, 12, av->fileHandle ) != 12 ) return -1;
	if ( buffer[0] != 0x45564157 || buffer[1] != 0x20746D66 ) return -1;
	
	u8* fmt_data = (u8*)malloc(buffer[2]);
	if ( !fmt_data ) return -1;

	if ( FileRead_read( fmt_data, buffer[2], av->fileHandle ) != buffer[2] ) {
		free(fmt_data);
		return -1;
	}
	
	u16 wave_type		= *((u16*)fmt_data);
	u16 channels		= *((u16*)(fmt_data + 2));
	av->sampleRate		= *((u32*)(fmt_data + 4));
	av->wav.bloack_size	= *((u32*)(fmt_data + 8));
	av->wav.blockalign	= *((u16*)(fmt_data + 12)); // 4
	u16 bit_per_sample	= *((u16*)(fmt_data + 14));
	av->sampleSize		= bit_per_sample / 8;
	av->channels		= channels;
	av->format			= DXP_SOUNDFMT_WAV;
	
	free(fmt_data);
	
	if ( wave_type != WAVE_FORMAT_PCM ) return -1;

	//dataチャンクまで進める
	if ( FileRead_read( buffer, 8, av->fileHandle ) != 8 ) return -1;

	while ( buffer[0] != 0x61746164 ) {
		FileRead_seek(av->fileHandle, buffer[1], SEEK_CUR);
		if ( FileRead_read( buffer, 8, av->fileHandle ) != 8 )
			return -1;
	}

	//波形データの開始位置、サイズ
	av->wav.dataPos = FileRead_tell(av->fileHandle);
	av->wav.dataSize = buffer[1];
	if ( !av->wav.dataSize ) return -1;
	
	//if ( av->wav.dataSize % av->wav.bloack_size != 0 ) return -1;
		
	av->nextPos = 0;
	av->outSampleNum = WAV_FRAME_SIZE;
	
	return 0;
}

/* -----------------------------------------------------------------
	WAVを読み込む(1ブロックごと)
 ----------------------------------------------------------------- */

int dxpSoundWavDecode(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_WAV ) return -1;

	FileRead_seek(av->fileHandle, av->wav.dataPos + av->nextPos * av->wav.blockalign, SEEK_SET);
	
	int samplecount = dxpSoundGetNextSampleNum(av);
	int read = samplecount * av->wav.blockalign;

	//memset(av->pcmOut, 0, WAV_FRAME_SIZE * av->wav.blockalign);
	if ( samplecount <= 0 ) return -1;

	if ( FileRead_read(av->pcmOut, read, av->fileHandle) != read ) return -1;

	av->nextPos += samplecount;
	return 0;
}

int dxpSoundWavAllDecode(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_WAV ) return -1;

	FileRead_seek(av->fileHandle, av->wav.dataPos, SEEK_SET);
	if ( FileRead_read(av->pcmOut, av->wav.dataSize, av->fileHandle) != av->wav.dataSize ) return -1;

	av->nextPos = 0;
	return 0;
}

/* -----------------------------------------------------------------
	終了処理
 ----------------------------------------------------------------- */

int dxpSoundWavEnd(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_WAV ) return -1;
	return 0;
}

/* -----------------------------------------------------------------
	総サンプル数
 ----------------------------------------------------------------- */

int dxpSoundWavGetSampleLength(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_WAV ) return -1;
	if ( av->wav.blockalign < 1 ) return -1;
	return av->wav.dataSize / av->wav.blockalign;
}

/* シーク */
int dxpSoundWavSeek(DXPAVCONTEXT *av, int sample)
{
	if ( av->format != DXP_SOUNDFMT_WAV ) return -1;
	FileRead_seek(av->fileHandle, av->wav.dataPos + sample * av->wav.blockalign, SEEK_SET);
	av->nextPos = sample;
	return 0;
}