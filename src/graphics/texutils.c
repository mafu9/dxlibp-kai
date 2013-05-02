#include <valloc.h>
#include <malloc.h>
#include <string.h>
#include <pspgu.h>
#include "dxppng.h"
#include"../graphics.h"
#include "../fileio.h"



DXPTEXTURE3* dxpGraphicsCreateTexture()
{
	u32 i;
	DXPTEXTURE3 *texptr;
	for(i = 0;i < DXP_BUILDOPTION_TEXTURE_MAXNUM;++i)
		if(dxpGraphicsData.texarray[i] == 0)break;
	if(i == DXP_BUILDOPTION_TEXTURE_MAXNUM)return NULL;
	texptr = (DXPTEXTURE3*)malloc(sizeof(DXPTEXTURE3));
	if(!texptr)return NULL;
	memset(texptr,0,sizeof(DXPTEXTURE3));
	texptr->thisptrptr = &dxpGraphicsData.texarray[i];
	dxpGraphicsData.texarray[i] = texptr;
	return texptr;
}

int dxpGraphicsReleseTexture(DXPTEXTURE3 *texptr)
{
	if(!texptr)return -1;
	if(texptr->refcount)return -1;
	free(texptr->ppalette);
	free(texptr->texdata);
	vfree(texptr->texvram);
	*texptr->thisptrptr = NULL;
	free(texptr);
	return 0;
}

DXPGRAPHICSHANDLE* dxpGraphicsCreateGraphicHandle()
{
	u32 i;
	DXPGRAPHICSHANDLE *gptr;
	for(i = 0;i < DXP_BUILDOPTION_GHANDLE_MAXNUM;++i)
		if(!dxpGraphicsData.grapharray[i])break;
	if(i == DXP_BUILDOPTION_GHANDLE_MAXNUM)return NULL;
	gptr = (DXPGRAPHICSHANDLE*)malloc(sizeof(DXPGRAPHICSHANDLE));
	if(!gptr)return NULL;
	memset(gptr,0,sizeof(DXPGRAPHICSHANDLE));
	gptr->handle = (int)i;
	dxpGraphicsData.grapharray[i] = gptr;
	return gptr;
}

int dxpGraphicsReleseGraphicHandle(DXPGRAPHICSHANDLE* gptr)
{
	if(!gptr)return -1;
	if(gptr->tex)
	{
		--gptr->tex->refcount;
		dxpGraphicsReleseTexture(gptr->tex);
	}
	dxpGraphicsData.grapharray[gptr->handle] = NULL;
	free(gptr);
	return 0;
}

int dxpGraphicsCalcTexSize(int width,int height,int psm)
{
	height = dxpN_2(height);
	switch(psm)
	{
	case GU_PSM_4444:
	case GU_PSM_5551:
	case GU_PSM_5650:
	case GU_PSM_T16:
		width = ((width + 7) >> 3) << 3;
		return width * height * 2;
	case GU_PSM_8888:
	case GU_PSM_T32:
		width = ((width + 3) >> 2) << 2;
		return width * height * 4;
	case GU_PSM_DXT1:
	case GU_PSM_T4:
		width = ((width + 31) >> 5) << 5;
		return width * height / 2;
	case GU_PSM_DXT3:
	case GU_PSM_DXT5:
	case GU_PSM_T8:
		width = ((width + 15) >> 4) << 4;
		return width * height;
	}
	return 0;
}

static void dxpGraphicsSwizzleFast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   const u8* ysrc = in;
   u32* dst = (u32*)out;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)//16byte幅で高さ8の情報を線形に転送
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

static void dxpGraphicsUnswizzleFast(u8* out, const u8* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   u8* ysrc = out;
   u32* dst = (u32*)in;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      u8* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         u32* src = (u32*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(src++) = *(dst++);
            *(src++) = *(dst++);
            *(src++) = *(dst++);
            *(src++) = *(dst++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}

int SwizzleGraph(int gh)
{
	GUINITCHECK;
	GUSYNC;
	u32 size;
	DXPTEXTURE3 *texptr = dxpGraphHandle2TexPtr(gh);
	if(!texptr)return -1;
	if(texptr->swizzledflag)return 0;
	size = dxpGraphicsCalcTexSize(texptr->pitch,texptr->height,texptr->psm);
	if(texptr->texvram)
	{
		if(!texptr->texdata)return -1;
		dxpGraphicsSwizzleFast(texptr->texdata,texptr->texvram,PSM2BYTEX2(texptr->psm) * texptr->pitch / 2,texptr->height);
		memcpy(texptr->texvram,texptr->texdata,size);
	}
	else
	{
		const int assertFlag = GetMemoryAssertFlag();
		void *buf;
		SetMemoryAssertFlag(FALSE);
		buf = malloc(size);
		SetMemoryAssertFlag(assertFlag);
		if(!buf)return -1;
		dxpGraphicsSwizzleFast(buf,texptr->texdata,PSM2BYTEX2(texptr->psm) * texptr->pitch / 2,texptr->height);
		memcpy(texptr->texdata,buf,size);
		free(buf);
	}
	sceKernelDcacheWritebackAll();
	texptr->swizzledflag = 1;
	texptr->reloadflag = 1;
	return 0;
}

int UnswizzleGraph(int gh)
{
	GUINITCHECK;
	GUSYNC;
	u32 size;
	DXPTEXTURE3 *texptr = dxpGraphHandle2TexPtr(gh);
	if(!texptr)return -1;
	if(!texptr->swizzledflag)return 0;
	size = dxpGraphicsCalcTexSize(texptr->pitch,texptr->height,texptr->psm);
	if(texptr->texvram)
	{
		if(!texptr->texdata)return -1;
		dxpGraphicsUnswizzleFast(texptr->texdata,texptr->texvram,PSM2BYTEX2(texptr->psm) * texptr->pitch / 2,texptr->height);
		memcpy(texptr->texvram,texptr->texdata,size);
	}
	else
	{
		const int assertFlag = GetMemoryAssertFlag();
		void *buf;
		SetMemoryAssertFlag(FALSE);
		buf = malloc(size);
		SetMemoryAssertFlag(assertFlag);
		if(!buf)return -1;
		dxpGraphicsUnswizzleFast(buf,texptr->texdata,PSM2BYTEX2(texptr->psm) * texptr->pitch / 2,texptr->height);
		memcpy(texptr->texdata,buf,size);
		free(buf);
	}
	sceKernelDcacheWritebackAll();
	texptr->swizzledflag = 0;
	texptr->reloadflag = 1;
	return 0;
}

int MoveGraphToVRAM(int gh)
{
	GUINITCHECK;
	GUSYNC;
	DXPTEXTURE3 *texptr;
	int size;
	texptr = dxpGraphHandle2TexPtr(gh);
	if(!texptr)return -1;
	if(texptr->texvram)return 0;
	size = dxpGraphicsCalcTexSize(texptr->pitch,texptr->height,texptr->psm);
	texptr->texvram = valloc(size);
	if(!texptr->texvram)return -1;
	memcpy(texptr->texvram,texptr->texdata,size);
	texptr->reloadflag = 1;
	sceKernelDcacheWritebackAll();
	return 0;
}

int MoveGraphToDDR(int gh)
{
	GUINITCHECK;
	GUSYNC;
//vramあり→vram上のデータを消す
//vramなし→なにもしない
	DXPTEXTURE3 *texptr;
	int size;
	texptr = dxpGraphHandle2TexPtr(gh);
	if(!texptr)return -1;
	if(!texptr->texvram)return 0;
	size = dxpGraphicsCalcTexSize(texptr->pitch,texptr->height,texptr->psm);
	memcpy(texptr->texdata,vCPUPointer(texptr->texvram),size);
	vfree(texptr->texvram);
	texptr->texvram = NULL;
	texptr->reloadflag = 1;
	sceKernelDcacheWritebackAll();
	return 0;
}

//int UpdateGraphToDDR(int gh)
//{
////vramなし→なにもしない
////vramあり→vramのデータをDDRにコピー
//	return -1;
//}
//
//int UpdateGraphToVRAM(int gh)
//{
////vramなし→なにもしない
////vramあり→DDRのデータをvramにコピー
//	return -1;
//}

/*
inline unsigned int AlignPow2( unsigned int a ) {
	unsigned int i = 1;
	while ( a > ( i <<= 1 ) ) { if ( !i ) break; }
	return i;
}
*/


static DXPTEXTURE3* dxpLoadBitmapImage(int fileHandle);
static DXPTEXTURE3* dxpLoadJpegImage(int fileHandle);
static DXPTEXTURE3* dxpLoadPngImage(int fileHandle);

int dxpGraphLoadFromFileHandle(int fileHandle)
{
	GUINITCHECK;
	DXPGRAPHICSHANDLE *gptr = NULL;
	DXPTEXTURE3 *texptr = NULL;

	if(fileHandle == 0)return -1;

	gptr = dxpGraphicsCreateGraphicHandle();
	if(!gptr)return -1;

	texptr = dxpLoadPngImage(fileHandle);
	if(texptr) goto success;
		
	texptr = dxpLoadJpegImage(fileHandle);
	if(texptr) goto success;
			
	texptr = dxpLoadBitmapImage(fileHandle);
	if(texptr) goto success;
	
	dxpGraphicsReleseGraphicHandle(gptr);
	return -1;

success:
	gptr->tex = texptr;
	++texptr->refcount;

	gptr->u0 = gptr->v0 = 0;
	gptr->u1 = texptr->umax;
	gptr->v1 = texptr->vmax;

	if(dxpGraphicsData.create_vram_graph)MoveGraphToVRAM(gptr->handle);
	if(dxpGraphicsData.create_swizzled_graph)SwizzleGraph(gptr->handle);

	sceKernelDcacheWritebackAll();

	return gptr->handle;
}

/* ---------------------------------------------
	Bitmap image
 --------------------------------------------- */

static DXPTEXTURE3* dxpLoadBitmapImage(int fileHandle)
{
	u8 buffer[BMP_HEADER_SIZE];

	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;

	//read bitmap header
	FileRead_seek(fileHandle, 0, SEEK_SET);
	if ( FileRead_read(buffer, BMP_HEADER_SIZE, fileHandle) != BMP_HEADER_SIZE ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	//windows bitmap
	s32 headersize;
	memcpy(&headersize, buffer + 14, sizeof(s32));
	if ( strncmp((const char*)buffer, "BM", 2) || headersize != 40 ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	//32bit or 24bit
	s32 bpp;
	memcpy(&bpp, buffer + 28, sizeof(s32));
	bpp = bpp >> 3;
	if ( !( bpp == 3 || bpp == 4 ) ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	//bitmap information
	s32 width, height;
	memcpy(&width, buffer + 18, sizeof(s32));
	memcpy(&height, buffer + 22, sizeof(s32));
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	int bufsize = pitch * texheight * 4;
	if ( width > 512 || width <= 0 || height > 512 || height <= 0 ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}
	u8 *data = (u8*)malloc(bufsize);
	if ( data == NULL ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}
	memset(data, 0, bufsize);

	//line size
	unsigned int line_size = width * bpp;
	if ( (line_size % 4) != 0 ) line_size = ((line_size >> 2) + 1) << 2;
	
	int x, y, i;
	u8 *linedata = (u8 *)malloc(line_size);

	if ( linedata == NULL ) {
		for ( y = 0, i = 0; y < height; ++y, i += (pitch - width) * 4 ) {
			FileRead_seek(fileHandle, BMP_HEADER_SIZE + line_size * (height - (y + 1)), SEEK_SET);
			for ( x = 0; x < width; ++x ) {
				FileRead_read(buffer, bpp, fileHandle);
				data[i++] = buffer[2];
				data[i++] = buffer[1];
				data[i++] = buffer[0];
				data[i++] = 0xFF;
			}
		}
	} else {
		for ( y = 0, i = 0; y < height; y++, i += (pitch - width) * 4 ) {
			FileRead_seek(fileHandle, BMP_HEADER_SIZE + line_size * (height - (y + 1)), SEEK_SET);
			FileRead_read(linedata, line_size, fileHandle);
			int a;
			for ( x = 0, a = 0; x < width; x++, a += bpp ) {
				data[i++] = linedata[a + 2];
				data[i++] = linedata[a + 1];
				data[i++] = linedata[a + 0];
				data[i++] = 0xFF;
			}
		}
		free(linedata);
	}

	texptr->texdata = (void *)data;
	texptr->alphabit = 0;
	texptr->colorkey = dxpGraphicsData.colorkey;
	texptr->width = texwidth;
	texptr->height = texheight;
	texptr->pitch = pitch;
	texptr->ppalette = NULL;
	texptr->psm = PSM_8888;
	texptr->reloadflag = 1;
	texptr->size2_nflag = (width == texwidth && height == texheight) ? 1 : 0;
	texptr->swizzledflag = 0;
	texptr->texvram = 0;
	texptr->umax = width;
	texptr->vmax = height;

	return texptr;
}

/* ---------------------------------------------
	Joint Photographic Experts Group
 --------------------------------------------- */

static int check_jpeg_header(int fileHandle);

#ifdef DXP_BUILDOPTION_USE_LIBJPEG

METHODDEF(void) jpeg_init_source(j_decompress_ptr cinfo)
{
	DXPJPEGSRCMGR *src = (DXPJPEGSRCMGR*)cinfo->src;
	src->startOfFile = TRUE;
}

METHODDEF(boolean) jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
	DXPJPEGSRCMGR *src = (DXPJPEGSRCMGR*)cinfo->src;
	int nbytes = FileRead_read(src->buffer, JPEG_INPUT_BUF_SIZE, src->fileHandle);
	if(nbytes <= 0)
	{
		if(src->startOfFile)ERREXIT(cinfo, JERR_INPUT_EMPTY);	/* Treat empty input file as fatal error */
		WARNMS(cinfo, JWRN_JPEG_EOF);
		/* Insert a fake EOI marker */
		src->buffer[0] = (JOCTET)0xFF;
		src->buffer[1] = (JOCTET)JPEG_EOI;
		nbytes = 2;
	}
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->startOfFile = FALSE;
	return TRUE;
}

METHODDEF(void) jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
	DXPJPEGSRCMGR *src = (DXPJPEGSRCMGR*)cinfo->src;
	/* Just a dumb implementation for now.  Could use fseek() except
	 * it doesn't work on pipes.  Not clear that being smart is worth
	 * any trouble anyway --- large skips are infrequent.
	 */
	if(num_bytes > 0)
	{
		while(num_bytes > (long) src->pub.bytes_in_buffer)
		{
			num_bytes -= (long) src->pub.bytes_in_buffer;
			jpeg_fill_input_buffer(cinfo);
			/* note we assume that fill_input_buffer will never return FALSE,
			 * so suspension need not be handled.
			 */
		}
		src->pub.next_input_byte += (size_t)num_bytes;
		src->pub.bytes_in_buffer -= (size_t)num_bytes;
	}
}

METHODDEF(void) jpeg_term_source(j_decompress_ptr cinfo)
{
	/* no work necessary here */
}

LOCAL(void) jpeg_dxpio_src(j_decompress_ptr cinfo, int fileHandle)
{
	DXPJPEGSRCMGR *src;
	/* The source object and input buffer are made permanent so that a series
	 * of JPEG images can be read from the same file by calling jpeg_stdio_src
	 * only before the first one.  (If we discarded the buffer at the end of
	 * one image, we'd likely lose the start of the next one.)
	 * This makes it unsafe to use this manager and a different source
	 * manager serially with the same JPEG object.  Caveat programmer.
	 */
	if (cinfo->src == NULL)	/* first time for this JPEG object? */
	{
		cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(DXPJPEGSRCMGR));
		src = (DXPJPEGSRCMGR*) cinfo->src;
		src->buffer = (JOCTET*)(*cinfo->mem->alloc_small)((j_common_ptr)cinfo, JPOOL_PERMANENT, JPEG_INPUT_BUF_SIZE * sizeof(JOCTET));
	}
	src = (DXPJPEGSRCMGR*) cinfo->src;
	src->pub.init_source = jpeg_init_source;
	src->pub.fill_input_buffer = jpeg_fill_input_buffer;
	src->pub.skip_input_data = jpeg_skip_input_data;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = jpeg_term_source;
	src->fileHandle = fileHandle;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

METHODDEF(void) jpeg_error_exit(j_common_ptr cinfo)
{
	DXPJPEGERRMGR *err = (DXPJPEGERRMGR*)cinfo->err;
	(*cinfo->err->output_message)(cinfo);
	longjmp(err->setjmp_buffer, 1);
}

#endif

static DXPTEXTURE3* dxpLoadJpegImage(int fileHandle)
{
#ifdef DXP_BUILDOPTION_USE_LIBJPEG
	
	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;

	int is_jpeg = check_jpeg_header(fileHandle);
	if ( !is_jpeg ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	FileRead_seek(fileHandle, 0, SEEK_SET);

	int y = 0, x = 0, i = 0;
	struct jpeg_decompress_struct cinfo;
	DXPJPEGERRMGR jerr;
	u8 *data = NULL, *linedata = NULL;

	//read header
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpeg_error_exit;
	if(setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		free(data);
		free(linedata);
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_dxpio_src(&cinfo, fileHandle);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_calc_output_dimensions(&cinfo);
	jpeg_start_decompress(&cinfo);

	//jpeg information
	int width = cinfo.output_width;
	int height = cinfo.output_height;
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	int bufsize = pitch * texheight * 4;
	if ( width > 512 || height > 512 ) {
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	data = (u8*)malloc(bufsize);
	linedata = (u8*)malloc(3 * width);
	if ( data == NULL || linedata == NULL ) {
		if ( data ) free(data);
		if ( linedata ) free(linedata);
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	memset(linedata, 0, 3 * width);
	memset(data, 0, bufsize);

	for ( i = 0, y = 0; y < height; y++, i += (pitch - width) * 4 ) {
		jpeg_read_scanlines(&cinfo, &linedata, 1);
		if(cinfo.out_color_components == 3) // RGB
		{
			for ( x = 0; x < width; x++ ) {
				data[i++] = linedata[x * 3 + 0];
				data[i++] = linedata[x * 3 + 1];
				data[i++] = linedata[x * 3 + 2];
				data[i++] = 0xFF;
			}
		}
		else // (cinfo.out_color_components == 1) // glayscale
		{
			for ( x = 0; x < width; x++ ) {
				data[i++] = linedata[x];
				data[i++] = linedata[x];
				data[i++] = linedata[x];
				data[i++] = 0xFF;
			}
		}
	}
	
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	free(linedata);

	texptr->texdata = (void *)data;
	texptr->alphabit = 0;
	texptr->colorkey = dxpGraphicsData.colorkey;
	texptr->width = texwidth;
	texptr->height = texheight;
	texptr->pitch = pitch;
	texptr->ppalette = NULL;
	texptr->psm = PSM_8888;
	texptr->reloadflag = 1;
	texptr->size2_nflag = (width == texwidth && height == texheight) ? 1 : 0;
	texptr->swizzledflag = 0;
	texptr->texvram = 0;
	texptr->umax = width;
	texptr->vmax = height;

	return texptr;
#endif

	return NULL;
}

const char jpeg_marker[9][2] = {
	{ 0xFF, 0xD8 }, //SOI
	{ 0xFF, 0xE0 }, //APP0
	{ 0xFF, 0xDB }, //DQT
	{ 0xFF, 0xC4 }, //DHT
	{ 0xFF, 0xC0 }, //SOF0
	{ 0xFF, 0xC1 }, //SOF1
	{ 0xFF, 0xDD }, //DRI
	{ 0xFF, 0xDA }, //SOS
	{ 0xFF, 0xD9 }, //EOI
};

static int check_jpeg_header(int fileHandle)
{
	char marker[4] = { 0, };

	if ( FileRead_seek(fileHandle, 0, SEEK_SET) != 0 ) return 0;
	if ( FileRead_read(marker, 4, fileHandle) != 4 ) return 0;
	if ( strncmp(marker, jpeg_marker[JPEG_MARKER_SOI], 2) != 0 ) return 0;

	/*
	char buf[4]
	unsigned short marker_size;
	while ( 1 ) {		
		if ( FileRead_read(marker, 2, fileHandle) != 2 ) return 0;
		if ( !strcmp(marker, jpeg_marker[JPEG_MARKER_APP0]) ) {
			if ( FileRead_read(&marker_size, 2, fileHandle) != 2 ) return 0;
			if ( FileRead_read(buf, 4, fileHandle) != 4 ) return 0;
			if ( strcmp(marker, "JFIF") != 0 ) return 0;
			if ( FileRead_seek(fileHandle, marker_size - 6, SEEK_CUR) != 0 ) return 0;
		} else if ( !strcmp(marker, jpeg_marker[JPEG_MARKER_SOS]) ) {
			break;
		} else {
			if ( FileRead_read(&marker_size, 2, fileHandle) != 2 ) return 0;
			if ( FileRead_seek(fileHandle, marker_size - 2, SEEK_CUR) != 0 ) return 0;
		}
	}
	*/

	return 1;
}

static DXPTEXTURE3* dxpLoadPngImage(int fileHandle)
{
	u32 filesize;
	DXPPNG png;
	DXPPNG_PARAMS params;
	void *buf = NULL;
	DXPTEXTURE3 *texptr = NULL;
	FileRead_seek(fileHandle, 0, SEEK_END);
	filesize = FileRead_tell(fileHandle);
	if(!filesize)return NULL;
	buf = malloc(filesize);
	texptr = dxpGraphicsCreateTexture();
	if(!buf || !texptr)
	{
		free(buf);
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}
	FileRead_seek(fileHandle, 0, SEEK_SET);
	FileRead_read(buf,filesize, fileHandle);
	params.funcs.pmalloc = malloc;
	params.funcs.pmemalign = memalign;
	params.funcs.pfree = free;
	params.mode = DXPPNG_MODE_GPU;
	params.src = buf;
	params.srcLength = filesize;
	if(dxppng_decode(&params,&png) == -1)
	{
		free(buf);
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}
	free(buf);
	texptr->alphabit = png.alpha ? 1 : 0;
	texptr->colorkey = dxpGraphicsData.colorkey;
	texptr->width = png.widthN2;
	texptr->height = png.heightN2;
	texptr->pitch = png.pitch;
	texptr->ppalette = (u32 *)png.clut;
	texptr->psm = png.psm;
	texptr->reloadflag = 1;
	texptr->size2_nflag = (png.height == png.heightN2 && png.width == png.widthN2 ? 1 : 0);
	texptr->swizzledflag = 0;
	texptr->texdata = png.raw;
	texptr->texvram = 0;
	texptr->umax = png.width;
	texptr->vmax = png.height;
	return texptr;
}

