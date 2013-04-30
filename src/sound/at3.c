#include "../sound.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <psputility.h>
#include <pspaudiocodec.h>

int At3_Sample_Per_Frame[2] = { 1024, 2048 };

/* -----------------------------------------------------------------
	AT3デコードの為の初期化
 ----------------------------------------------------------------- */

int dxpSoundAt3Init(DXPAVCONTEXT *av)
{
	u32 buffer[3] = { 0, 0, 0 };
	int status;

	//RIFFヘッダ（buffer[1] -> ファイルサイズ - 8）
	FileRead_seek(av->fileHandle, 0, SEEK_SET);
	if ( FileRead_read( buffer, 8, av->fileHandle ) != 8 ) return -1;
	if ( buffer[0] != 0x46464952 ) return -1;
	
	//WAVEヘッダ
	if ( FileRead_read( buffer, 12, av->fileHandle ) != 12 ) return -1;
	if ( buffer[0] != 0x45564157 || buffer[1] != 0x20746D66 ) return -1;
	
	u8* fmt_data = (u8*)malloc(buffer[2]);
	if ( !fmt_data ) return -1;

	//fmtチャンク
	if ( FileRead_read( fmt_data, buffer[2], av->fileHandle ) != buffer[2] ) {
		free(fmt_data);
		return -1;
	}
	
	u16 wave_type		= *((u16*)fmt_data);
	u16 channels		= *((u16*)(fmt_data + 2));
	av->at3.data_align	= *((u16*)(fmt_data + 12));
	av->sampleRate		= *((u32*)(fmt_data + 4));
	u16 bit_per_sample	= *((u16*)(fmt_data + 14));
	av->format			= DXP_SOUNDFMT_AT3;
		
	if ( wave_type == WAVE_FORMAT_ATRAC3 ) {
		av->at3.type = AT3_TYPE_ATRAC3;
		av->at3.samples_per_frame = 1024;
	} else if ( wave_type == WAVE_FORMAT_ATRAC3PLUS ) {
		av->at3.type = AT3_TYPE_ATRAC3PLUS;
		av->at3.at3plus_flagdata[0] = fmt_data[42];
		av->at3.at3plus_flagdata[1] = fmt_data[43];
		av->at3.samples_per_frame = 2048;
	} else {
		return -1;
	}

	av->sampleSize = bit_per_sample / 8;
	av->channels = channels;
	av->nextPos = 0;
	av->outSampleNum = av->at3.samples_per_frame;

	free(fmt_data);

	//dataチャンクまで進める
	if ( FileRead_read( buffer, 8, av->fileHandle ) != 8 ) return -1;

	while ( buffer[0] != 0x61746164 ) {
		FileRead_seek(av->fileHandle, buffer[1], SEEK_CUR);
		if ( FileRead_read( buffer, 8, av->fileHandle ) != 8 )
			return -1;
	}

	//波形データの開始位置、サイズ
	av->at3.dataPos = FileRead_tell(av->fileHandle);
	av->at3.dataSize = buffer[1];
	if ( !av->at3.dataSize ) return -1;

	//AT3にはサンプル毎に8バイトのヘッダが付けられているものと、そうでないものがある
	if ( av->at3.dataSize % av->at3.data_align != 0 ) {
		if ( av->at3.dataSize % (av->at3.data_align + 8) != 0 ) 
			return -1;
		else
			av->at3.dataHeader = 1;
	} else {
		av->at3.dataHeader = 0;
	}

	//デコードのためのバッファ確保
	av->at3.codecBuf = (unsigned long*)memalign(64, sizeof(u32) * 65);
	if ( !av->at3.codecBuf ) return -1;
	memset(av->at3.codecBuf, 0, sizeof(u32) * 65);

	if ( av->at3.type == AT3_TYPE_ATRAC3 ) {
		/*
		 * ATRAC3のビットレート : フレームサイズ (av->at3.data_align)
		 *
		 * -  66kbps : 0xC0
		 * -  94kbps : 0x110
		 * - 105kbps : 0x130
		 * - 132kbps : 0x180
		 * - 146kbps : 0x1A8
		 * - 176kbps : 0x200
		 * - 264kbps : 0x300
		 * - 352kbps : 0x400
		*/

		if ( av->at3.data_align == 0xC0 ) {
			av->at3.at3_channel_mode = 0x1;
		} else {
			av->at3.at3_channel_mode = 0x0;
		}

		//align to 64 byte
		av->at3.dataBufSize = av->at3.data_align;
		int temp_size = av->at3.dataBufSize;
		int mod_64 = temp_size & 0x3F;
		if ( mod_64 != 0 ) temp_size += 64 - mod_64;
		av->at3.dataBuf = (u8*)memalign(64, temp_size);
		av->nextPos = 0;
		av->outSampleNum = 1024;
		if ( !av->at3.dataBuf ) {
			free(av->at3.codecBuf);
			return -1;
		}

		av->at3.codecBuf[26] = 0x20;

		status = sceAudiocodecCheckNeedMem(av->at3.codecBuf, PSP_CODEC_AT3PLUS);
		if ( status < 0 ) {
			free(av->at3.codecBuf);
			free(av->at3.dataBuf);
			return -1;
		}

		status = sceAudiocodecGetEDRAM(av->at3.codecBuf, PSP_CODEC_AT3PLUS);
		if ( status < 0 ) {
			free(av->at3.codecBuf);
			free(av->at3.dataBuf);
			return -1;
		}

		av->at3.codecBuf[10] = 4;
		av->at3.codecBuf[44] = 2;

		status = sceAudiocodecInit(av->at3.codecBuf, PSP_CODEC_AT3PLUS);
		if ( status < 0 ) {
			sceAudiocodecReleaseEDRAM(av->at3.codecBuf);
			free(av->at3.codecBuf);
			free(av->at3.dataBuf);
			return -1;
		}

	} else if ( av->at3.type == AT3_TYPE_ATRAC3PLUS ) {
		/*
		 * ATRAC3PLUSのビットレート : フレームサイズ (av->at3.data_align)
		 *
		 * -  44kbps : 0x118
		 * -  64kbps : 0x178
		 * -  96kbps : 0x230
		 * - 128kbps : 0x2E8
		 * - 160kbps : 0x3A8
		 * - 192kbps : 0x460
		 * - 256kbps : 0x5D0
		 * - 320kbps : 0x748
		 * - 352kbps : 0x800
		*/

		//align to 64 byte
		av->at3.dataBufSize = av->at3.data_align + 8;
		int temp_size = av->at3.data_align + 8;
		int mod_64 = temp_size & 0x3F;
		if ( mod_64 != 0 ) temp_size += 64 - mod_64;
		av->at3.dataBuf = (u8*)memalign(64, temp_size);
		av->nextPos = 0;
		av->outSampleNum = 2048;
		if ( !av->at3.dataBuf ) {
			free(av->at3.codecBuf);
			return -1;
		}

		av->at3.codecBuf[5] = 0x1;
		av->at3.codecBuf[10] = av->at3.at3plus_flagdata[1];
		av->at3.codecBuf[10] = ( av->at3.codecBuf[10] << 8 ) | av->at3.at3plus_flagdata[0];
		av->at3.codecBuf[12] = 0x1;
		av->at3.codecBuf[14] = 0x1;

		status = sceAudiocodecCheckNeedMem(av->at3.codecBuf, PSP_CODEC_AT3PLUS);
		if ( status < 0 ) {
			free(av->at3.codecBuf);
			free(av->at3.dataBuf);
			return -1;
		}

		status = sceAudiocodecGetEDRAM(av->at3.codecBuf, PSP_CODEC_AT3PLUS);
		if ( status < 0 ) {
			free(av->at3.codecBuf);
			free(av->at3.dataBuf);
			return -1;
		}

		status = sceAudiocodecInit(av->at3.codecBuf, PSP_CODEC_AT3PLUS);
		if ( status < 0 ) {
			sceAudiocodecReleaseEDRAM(av->at3.codecBuf);
			free(av->at3.codecBuf);
			free(av->at3.dataBuf);
			return -1;
		}
	}
	
	return 0;
}

/* -----------------------------------------------------------------
	AT3をデコードする
 ----------------------------------------------------------------- */
int dxpSoundAt3Decode(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_AT3 ) return -1;

	u32 decode_format = 0;
	int position = av->nextPos / av->at3.samples_per_frame * ( av->at3.dataHeader ? av->at3.data_align + 8 : av->at3.data_align );
	FileRead_seek(av->fileHandle, av->at3.dataPos + position, SEEK_SET);

	//ATRAC3
	if ( av->at3.type == AT3_TYPE_ATRAC3 ) {
		memset( av->at3.dataBuf, 0, av->at3.dataBufSize );

		if ( FileRead_read(av->at3.dataBuf, av->at3.data_align, av->fileHandle) != av->at3.data_align ) return -1;

		if ( av->at3.at3_channel_mode )
			memcpy(av->at3.dataBuf + av->at3.data_align, av->at3.dataBuf, av->at3.data_align);

		decode_format = PSP_CODEC_AT3;
	//ATRAC3PLUS
	} else {
		memset( av->at3.dataBuf, 0, av->at3.dataBufSize + 8 );

		//ヘッダ無し波形データ
		if ( av->at3.dataHeader == 0 ) {
			av->at3.dataBuf[0] = 0x0F;
			av->at3.dataBuf[1] = 0xD0;
			av->at3.dataBuf[2] = av->at3.at3plus_flagdata[0];
			av->at3.dataBuf[3] = av->at3.at3plus_flagdata[1];			
			if ( FileRead_read(av->at3.dataBuf + 8, av->at3.data_align, av->fileHandle) != av->at3.data_align ) return -1;
		} else {
			if ( FileRead_read(av->at3.dataBuf, av->at3.data_align + 8, av->fileHandle) != av->at3.data_align + 8 ) return -1;
		}

		decode_format = PSP_CODEC_AT3PLUS;
	}

	av->at3.codecBuf[6] = (unsigned long)av->at3.dataBuf;
	av->at3.codecBuf[8] = (unsigned long)av->pcmOut;

	if ( sceAudiocodecDecode(av->at3.codecBuf, decode_format) < 0 ) return -1;

	av->nextPos += av->at3.samples_per_frame;

	return 0;
}


/* -----------------------------------------------------------------
	終了処理
 ----------------------------------------------------------------- */
int dxpSoundAt3End(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_AT3 ) return -1;
	sceAudiocodecReleaseEDRAM(av->at3.codecBuf);
	free(av->at3.codecBuf);
	free(av->at3.dataBuf);
	return 0;
}

/* -----------------------------------------------------------------
	総サンプル数
 ----------------------------------------------------------------- */
int dxpSoundAt3GetSampleLength(DXPAVCONTEXT *av)
{
	if ( av->format != DXP_SOUNDFMT_AT3 ) return -1;

	if ( av->at3.dataHeader == 1 ) {
		return  av->at3.dataSize / (av->at3.data_align + 8) * av->at3.samples_per_frame;
	} else {
		return av->at3.dataSize / av->at3.data_align * av->at3.samples_per_frame;
	}
}

int dxpSoundAt3Seek(DXPAVCONTEXT *av, int sample)
{
	if ( av->format != DXP_SOUNDFMT_AT3 ) return -1;
	int position = sample / av->at3.samples_per_frame * ( av->at3.dataHeader ? av->at3.data_align + 8 : av->at3.data_align );
	FileRead_seek(av->fileHandle, av->at3.dataPos + position, SEEK_SET);
	av->nextPos = sample;
	return 0;
}