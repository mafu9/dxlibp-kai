#include "dxppng.h"
#include "../graphics.h"

#include <malloc.h>
#include <pspdebug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static DXPTEXTURE3* LoadMemoryBitmapImage(void *buffer, unsigned int len);
static DXPTEXTURE3* LoadMemoryJpegImage(void *buffer, unsigned int len);
static DXPTEXTURE3* LoadMemoryPngImage(void *buffer, unsigned int len);

int LoadMemoryGraph(void* data, unsigned int size)
{
	GUINITCHECK;
	if ( data == NULL || !size ) return -1;

	DXPGRAPHICSHANDLE *gptr = dxpGraphicsCreateGraphicHandle();
	if ( !gptr ) return -1;

	DXPTEXTURE3 *texptr = NULL;
	
	texptr = LoadMemoryPngImage(data, size);
	if ( texptr != NULL ) goto success;
			
	texptr = LoadMemoryJpegImage(data, size);
	if ( texptr != NULL ) goto success;
		
	texptr = LoadMemoryBitmapImage(data, size);
	if ( texptr != NULL ) goto success;
				
	dxpGraphicsReleseGraphicHandle(gptr);
	return -1;

success:
	
	gptr->tex = texptr;
	++texptr->refcount;

	gptr->u0 = gptr->v0 = 0;
	gptr->u1 = texptr->umax;
	gptr->v1 = texptr->vmax;

	if ( dxpGraphicsData.create_vram_graph ) MoveGraphToVRAM(gptr->handle);
	if ( dxpGraphicsData.create_swizzled_graph ) SwizzleGraph(gptr->handle);

	sceKernelDcacheWritebackAll();

	return gptr->handle;
}

/* ---------------------------------------------
	BITMAP
 --------------------------------------------- */

static DXPTEXTURE3* LoadMemoryBitmapImage(void *buffer, unsigned int len)
{
	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;

	int bpp, x, y, i;
	
	//windows bitmap
	memcpy(&bpp, buffer + 14, sizeof(int));
	if ( strncmp((char *)buffer, "BM", 2) || bpp != 40 ) goto err;

	//32bit or 24bit
	memcpy(&bpp, buffer + 28, sizeof(int));
	bpp = bpp >> 3;
	if ( !( bpp == 3 || bpp == 4 ) ) goto err;

	//bitmap information
	int width, height, pitch, texwidth, texheight, bufsize;
	memcpy(&width , buffer + 18, sizeof(int));
	memcpy(&height, buffer + 22, sizeof(int));
	pitch = ((width + 3) >> 2) << 2;
	texwidth = AlignPow2(width);
	texheight = AlignPow2(height);
	bufsize = pitch * texheight * 4;

	if ( width > 512 || width <= 0 || height > 512 || height <= 0 ) goto err;
	u8 *data = (u8*)malloc(bufsize);
	if ( data == NULL ) goto err;
	memset(data, 0, bufsize);
	
	//line size
	unsigned int line_size = width * bpp;
	if ( (line_size % 4) != 0 ) line_size = ((line_size >> 2) + 1) << 2;

	//read
	for ( y = 0, i = 0; y < height; ++y, i += (pitch - width) * 4 ) {
		u8 *src = buffer + (BMP_HEADER_SIZE + line_size * ( height - (y + 1) ) );
		int a = 0;
		for ( x = 0, a = 0; x < width; x++, a += bpp ) {
			data[i++] = src[a + 2];
			data[i++] = src[a + 1];
			data[i++] = src[a + 0];
			data[i++] = 0xFF;
		}
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

err:
	dxpGraphicsReleseTexture(texptr);
	return NULL;
}

/* ---------------------------------------------
	Joint Photographic Experts Group
 --------------------------------------------- */

int check_jpeg_header_mem(void *buffer);

#ifdef DXP_BUILDOPTION_USE_LIBJPEG

void my_exit(j_common_ptr cinfo)
{
	return;
}

#endif

static DXPTEXTURE3* LoadMemoryJpegImage(void *buffer, unsigned int len)
{
#ifdef DXP_BUILDOPTION_USE_LIBJPEG
	
	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;

	int is_jpeg = check_jpeg_header_mem(buffer);
	if ( !is_jpeg ) goto err;
	
	int y = 0, x = 0, i = 0;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	//read header
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = my_exit;
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, (unsigned char *)buffer, len);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	//jpeg information
	int width = cinfo.output_width;
	int height = cinfo.output_height;
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	if ( width > 512 || width <= 0 || height > 512 || height <= 0 ) goto err;
	int bufsize = pitch * texheight * 4;
	
	u8* data = (u8*)malloc(bufsize);
	u8* linedata = (u8*)malloc(3 * width);

	if ( data == NULL || linedata == NULL ) {
		if ( data != NULL ) free(data);
		if ( linedata != NULL ) free(linedata);
		jpeg_abort_decompress(&cinfo);
		goto err;
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

err:
	dxpGraphicsReleseTexture(texptr);

#endif

	return NULL;
}

int check_jpeg_header_mem(void *buffer)
{
	if ( !buffer ) return 0;
	
	char *src = (char *)buffer;

	if ( strncmp(src, jpeg_marker[JPEG_MARKER_SOI], 2) != 0 ) return 0;

	/*
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

	return 1;
}

#ifdef DXP_BUILDOPTION_USE_LIBPNG

typedef struct _my_png_buffer {
	unsigned char *data;
	unsigned int data_len;
	unsigned int data_offset;
} my_png_buffer;

void png_mem_read_proc(png_structp png_ptr, png_bytep buf, png_size_t size)
{
	my_png_buffer *png_buff = (my_png_buffer *)png_get_io_ptr(png_ptr);

	if ( png_buff->data_offset + size <= png_buff->data_len ) {
		memcpy(buf, png_buff->data + png_buff->data_offset, size);
		png_buff->data_offset += size;
	} else {
		png_error(png_ptr, "png_mem_read_func failed");
	}
}

void png_init_mem(png_structp png_ptr, my_png_buffer *png_buff)
{
	png_set_read_fn(png_ptr, (png_voidp)png_buff, (png_rw_ptr)png_mem_read_proc);
}

#endif

static DXPTEXTURE3* LoadMemoryPngImage(void *buffer, unsigned int len)
{
#ifdef DXP_BUILDOPTION_USE_LIBPNG

	DXPTEXTURE3 *texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) return NULL;
	
	my_png_buffer png_buff;
	png_buff.data = (unsigned char *)buffer;
	png_buff.data_len = len;
	png_buff.data_offset = 0;

	int is_png = png_check_sig((png_bytep)png_buff.data, 8);
	if ( !is_png ) goto err1;

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if ( png_ptr == NULL || info_ptr == NULL ) goto err1;
	
	//readä÷êîÇéwíË
	png_init_mem(png_ptr, &png_buff);
		
	//âÊëúèÓïÒÇì«çûÇﬁ
	png_uint_32 width, height;
	int bit_depth, color_type;
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
	png_set_expand(png_ptr);

	if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )
		png_set_tRNS_to_alpha(png_ptr);
	if ( color_type == PNG_COLOR_TYPE_PALETTE )
		png_set_expand(png_ptr);
	if ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
		png_set_expand(png_ptr);
	if ( bit_depth > 8 )
		png_set_strip_16(png_ptr);
	if ( color_type == PNG_COLOR_TYPE_GRAY )
		png_set_gray_to_rgb(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

	//jpeg information
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	if ( width > 512 || height > 512 ) goto err2;
	int bufsize = pitch * texheight * 4;

	u32 *data = (u32 *)malloc(bufsize);
	u32 *linedata = (u32 *)malloc(4 * width);
	if ( data == NULL || linedata == NULL ) {
		if ( data ) free(data);
		if ( linedata ) free(linedata);
		goto err2;
	}

	memset(linedata, 0, 4 * width);
	memset(data, 0, bufsize);

	int y, x;
	for ( y = 0; y < height; y++ ) {
		png_read_row(png_ptr, (u8*)linedata, NULL);
		for ( x = 0; x < width; x++ ) {
			data[x + y * pitch] = linedata[x];
		}
	}

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);	

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

err2:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);	

err1:
	dxpGraphicsReleseTexture(texptr);

#endif
	return NULL;
}
