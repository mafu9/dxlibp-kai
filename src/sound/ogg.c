#include "../sound.h"
#include <string.h>
#include <malloc.h>

static size_t ogg_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	int fh = *(int*)datasource;
	return (size_t)FileRead_read(ptr,(int)(size*nmemb),fh);
}

static int ogg_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	int fh = *(int*)datasource;
	return FileRead_seek(fh,(int)offset,whence);
}

static int ogg_close_func(void *datasource)
{
}

static long ogg_tell_func(void *datasource)
{
	int fh = *(int*)datasource;
	return (long)FileRead_tell(fh);
}

int dxpSoundOggInit(DXPAVCONTEXT *av)
{
	ov_callbacks callbacks;
	vorbis_info *vi;
	int ret;
	av->ogg.file = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));
	if(!av->ogg.file)return -1;
	memset(av->ogg.file,0,sizeof(OggVorbis_File));
	callbacks.read_func = ogg_read_func;
	callbacks.seek_func = ogg_seek_func;
	callbacks.close_func = ogg_close_func;
	callbacks.tell_func = ogg_tell_func;
	FileRead_seek(av->fileHandle, 0, SEEK_SET);
	ret = ov_open_callbacks(&av->fileHandle,av->ogg.file,NULL,0,callbacks);
	if(ret < 0)
	{
		free(av->ogg.file);
		return -1;
	}
	vi = ov_info(av->ogg.file, -1);
	if(!vi || (vi->channels != 1 && vi->channels != 2))
	{
		ov_clear(av->ogg.file);
		free(av->ogg.file);
		return -1;
	}
	av->sampleSize = 2;
	av->channels = (u8)vi->channels;
	av->sampleRate = (int)vi->rate;
	av->nextPos = (int)ov_pcm_tell(av->ogg.file);
	av->outSampleNum = 1024;
	av->format = DXP_SOUNDFMT_OGG;
	return 0;
}

int dxpSoundOggSeek(DXPAVCONTEXT *av,int sample)
{
	int ret;
	if(av->format != DXP_SOUNDFMT_OGG)return -1;
	ret = ov_pcm_seek(av->ogg.file,(ogg_int64_t)sample);
	if(ret < 0)return -1;
	av->nextPos = (int)ov_pcm_tell(av->ogg.file);
	return 0;
}

int dxpSoundOggDecode(DXPAVCONTEXT *av)
{
	int len = dxpSoundCalcBufferSize(av, dxpSoundGetNextSampleNum(av));
//	int readbytes = 0;
	int ret, bitstream;
	vorbis_info *vi;
	u8 *pcmOut = (u8*)av->pcmOut;
	if(av->format != DXP_SOUNDFMT_OGG)return -1;
	while(len > 0)
	{
		ret = ov_read(av->ogg.file,(char*)pcmOut,len,&bitstream);
		if(ret == OV_HOLE)continue;
		if(ret == OV_EBADLINK)return -1;
		vi = ov_info(av->ogg.file, -1);
		if(!vi || vi->channels != av->channels)return -1; // error
		pcmOut += ret;
//		readbytes += ret;
		len -= ret;
		if(ret == 0) len = 0; // EOF
	}
//	av->nextPos += readbytes / dxpSoundCalcBufferSize(av, 1);
	av->nextPos = (int)ov_pcm_tell(av->ogg.file);
	return 0;
}

int dxpSoundOggEnd(DXPAVCONTEXT *av)
{
	if(av->format != DXP_SOUNDFMT_OGG)return -1;
	ov_clear(av->ogg.file);
	free(av->ogg.file);
	return 0;
}

int dxpSoundOggGetSampleLength(DXPAVCONTEXT *av)
{
	int len;
	if(av->format != DXP_SOUNDFMT_OGG)return -1;
	len = (int)ov_pcm_total(av->ogg.file, -1);
	if(len == OV_EINVAL)return -1;
	return len;
}

