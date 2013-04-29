#include "dxppng.h"
#include "../graphics.h"

#include <malloc.h>
#include <pspdebug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static DXPTEXTURE3* LoadPngImage(const char *FileName);
static DXPTEXTURE3* LoadJpegImage(const char *FileName);
static DXPTEXTURE3* LoadBitmapImage(const char *FileName);

int LoadGraph(const char *FileName)
{
	if ( FileName == NULL ) return -1;
	GUINITCHECK;
	DXPGRAPHICSHANDLE *gptr = NULL;
	DXPTEXTURE3 *texptr = NULL;
	gptr = dxpGraphicsCreateGraphicHandle();
	if(!gptr)return -1;
	
#if 0
	//Šg’£Žq‚Å”»•Ê
	const char *ext = strrchr(FileName, '.');
	if ( !stricmp(ext,".png") ) {
		texptr = LoadPngImage(FileName);
	} else if ( !stricmp(ext,".jpg") || !stricmp(ext,".jpeg") )	{
		texptr = LoadJpegImage(FileName);
	} else if ( !strcmp(ext,".bmp") ) {
		texptr = LoadBitmapImage(FileName);
	} else {
		dxpGraphicsReleseGraphicHandle(gptr);
		return -1;
	}

	if ( !texptr ) {
		dxpGraphicsReleseGraphicHandle(gptr);
		return -1;
	}
#endif

	texptr = LoadPngImage(FileName);
	if ( texptr ) goto success;
		
	texptr = LoadJpegImage(FileName);
	if ( texptr ) goto success;
			
	texptr = LoadBitmapImage(FileName);
	if ( texptr ) goto success;
	
	dxpGraphicsReleseGraphicHandle(gptr);
	return -1;

success:
	
	gptr->tex = texptr;
	++texptr->refcount;

	gptr->u0 = gptr->v0 = 0;
	gptr->u1 = texptr->umax;
	gptr->v1 = texptr->vmax;

	if (dxpGraphicsData.create_vram_graph) MoveGraphToVRAM(gptr->handle);
	if (dxpGraphicsData.create_swizzled_graph) SwizzleGraph(gptr->handle);

	sceKernelDcacheWritebackAll();

	return gptr->handle;
}

/* ---------------------------------------------
	Bitmap image
 --------------------------------------------- */

static DXPTEXTURE3* LoadBitmapImage(const char *FileName)
{
	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;

	//open file
	FILE *fp = fopen(FileName, "rb");
	if ( fp == NULL ) goto err;

	char *buffer = (char *)malloc(BMP_HEADER_SIZE);
	if ( !buffer ) goto err;

	//read bitmap header
	if ( fread(buffer, sizeof(char), BMP_HEADER_SIZE, fp) != BMP_HEADER_SIZE ) goto err;

	//windows bitmap
	int headersize;
	memcpy(&headersize, buffer + 14, sizeof(int));
	if ( strncmp(buffer, "BM", 2) || headersize != 40 ) goto err;

	//32bit or 24bit
	int bpp;
	memcpy(&bpp, buffer + 28, sizeof(int));
	bpp = bpp >> 3;
	if ( !( bpp == 3 || bpp == 4 ) ) goto err;

	//bitmap information
	int width, height;
	memcpy(&width, buffer + 18, sizeof(int));
	memcpy(&height, buffer + 22, sizeof(int));
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	int bufsize = pitch * texheight * 4;
	if ( width > 512 || width <= 0 || height > 512 || height <= 0 ) goto err;
	u8 *data = (u8*)malloc(bufsize);
	if ( data == NULL ) goto err;
	memset(data, 0, bufsize);

	//line size
	unsigned int line_size = width * bpp;
	if ( (line_size % 4) != 0 ) line_size = ((line_size >> 2) + 1) << 2;
	
	int x, y, i;
	u8 *linedata = (u8 *)malloc(line_size);

	if ( linedata == NULL ) {
		for ( y = 0, i = 0; y < height; ++y, i += (pitch - width) * 4 ) {
			fseek(fp, BMP_HEADER_SIZE + line_size * (height - (y + 1)), SEEK_SET);
			for ( x = 0; x < width; ++x ) {
				fread(buffer, sizeof(char), bpp, fp);
				data[i++] = buffer[2];
				data[i++] = buffer[1];
				data[i++] = buffer[0];
				data[i++] = 0xFF;
			}
		}
	} else {
		for ( y = 0, i = 0; y < height; y++, i += (pitch - width) * 4 ) {
			fseek(fp, BMP_HEADER_SIZE + line_size * (height - (y + 1)), SEEK_SET);
			fread(linedata, sizeof(u8), line_size, fp);
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

	free(buffer);

	//close file
	fclose(fp);

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

err:
	if ( buffer ) free(buffer);
	if ( fp ) fclose(fp);
	dxpGraphicsReleseTexture(texptr);
	return NULL;
}

/* ---------------------------------------------
	Joint Photographic Experts Group
 --------------------------------------------- */

int check_jpeg_header_io(FILE *fp);

static DXPTEXTURE3* LoadJpegImage(const char *FileName)
{
#ifdef DXP_BUILDOPTION_USE_LIBJPEG
	
	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;

	int y = 0, x = 0, i = 0;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	//open file
	FILE *fp = fopen(FileName, "rb");
	if ( fp == NULL ) goto err;

	int is_jpeg = check_jpeg_header_io(fp);
	if ( !is_jpeg ) goto err;
	
	//read header
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	//jpeg information
	int width = cinfo.output_width;
	int height = cinfo.output_height;
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	int bufsize = pitch * texheight * 4;
	if ( width > 512 || height > 512 || cinfo.out_color_components == 1 ) {
		jpeg_abort_decompress(&cinfo);
		goto err;
	}

	u8* data = (u8*)malloc(bufsize);
	u8* linedata = (u8*)malloc(3 * width);
	if ( data == NULL || linedata == NULL ) {
		if ( data ) free(data);
		if ( linedata ) free(linedata);
		jpeg_abort_decompress(&cinfo);
		goto err;
	}

	memset(linedata, 0, 3 * width);
	memset(data, 0, bufsize);

	for ( i = 0, y = 0; y < height; y++, i += (pitch - width) * 4 ) {
		jpeg_read_scanlines(&cinfo, &linedata, 1);
		for ( x = 0; x < width; x++ ) {
			data[i++] = linedata[x * 3 + 0];
			data[i++] = linedata[x * 3 + 1];
			data[i++] = linedata[x * 3 + 2];
			data[i++] = 0xFF;
		}
	}
	
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	free(linedata);

	//close file
	fclose(fp);

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

err:
	if ( fp ) fclose(fp);
	dxpGraphicsReleseTexture(texptr);

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

int check_jpeg_header_io(FILE *fp)
{
	if ( !fp ) return 0;
	
	char marker[4] = { 0, };

	if ( fseek(fp, 0, SEEK_SET) != 0 ) return 0;
	if ( fread(marker, sizeof(char), 4, fp) != 4 ) return 0;
	if ( strncmp(marker, jpeg_marker[JPEG_MARKER_SOI], 2) != 0 ) return 0;

	/*
	char buf[4]
	unsigned short marker_size;
	while ( 1 ) {		
		if ( fread(marker, sizeof(char), 2, fp) != 2 ) return 0;
		if ( !strcmp(marker, jpeg_marker[JPEG_MARKER_APP0]) ) {
			if ( fread(&marker_size, sizeof(char), 2, fp) != 2 ) return 0;
			if ( fread(buf, sizeof(char), 4, fp) != 4 ) return 0;
			if ( strcmp(marker, "JFIF") != 0 ) return 0;
			if ( fseek(fp, marker_size - 6, SEEK_CUR) != 0 ) return 0;
		} else if ( !strcmp(marker, jpeg_marker[JPEG_MARKER_SOS]) ) {
			break;
		} else {
			if ( fread(&marker_size, sizeof(char), 2, fp) != 2 ) return 0;
			if ( fseek(fp, marker_size - 2, SEEK_CUR) != 0 ) return 0;
		}
	}
	*/

	fseek(fp, 0, SEEK_SET);
	return 1;
}


static DXPTEXTURE3* LoadPngImage(const char *FileName)
{
	u32 filesize;
	int fp = -1;
	DXPPNG png;
	DXPPNG_PARAMS params;
	void *buf = NULL;
	DXPTEXTURE3 *texptr = NULL;
	filesize = FileRead_size(FileName);
	if(!filesize)goto err;
	buf = malloc(filesize);
	if(!buf)goto err;
	texptr = dxpGraphicsCreateTexture();
	if(!texptr)goto err;
	fp = FileRead_open(FileName,0);
	if(fp == 0)goto err;
	FileRead_read(buf,filesize,fp);
	FileRead_close(fp);
	params.funcs.pmalloc = malloc;
	params.funcs.pmemalign = memalign;
	params.funcs.pfree = free;
	params.mode = DXPPNG_MODE_GPU;
	params.src = buf;
	params.srcLength = filesize;
	if(dxppng_decode(&params,&png) == -1)goto err;
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

err:
	FileRead_close(fp);
	free(buf);
	dxpGraphicsReleseTexture(texptr);
	return NULL;
}
