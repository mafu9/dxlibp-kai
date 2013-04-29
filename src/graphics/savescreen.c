#include "../graphics.h"
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <stdlib.h>

//JPEG保存時の設定
JPEGSETTING dxpjpegsetting = {
	100,
	0,
};

//DXPの内部でディスプレイバッファのアドレスを記録してた気がするけど...

int SaveScreen(const char *filename, int type)
{
	GUINITCHECK;

	sceIoRemove(filename);
	FILE *fp = fopen(filename, "wb");
    if ( fp == NULL ) return -1;

	u32* vram32 = NULL;
	u16* vram16 = NULL;
	int bufferwidth, pixelformat;

	sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, 0);
	vram16 = (u16*)vram32;

	switch(type) {

#ifdef DXP_BUILDOPTION_USE_LIBPNG

	case DXP_SAVE_TYPE_PNG:
		{
			int x, y, i;
			png_structp png_ptr;
			png_infop info_ptr;

			png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png_ptr) { fclose(fp); return -1; }
			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr) {
				png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
				fclose(fp);
				return -1;
			}
			png_init_io(png_ptr, fp);
			png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT,
				8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_write_info(png_ptr, info_ptr);

			u8* linebuffer = (u8*) malloc(SCREEN_WIDTH * 3);
			if ( !linebuffer ) { fclose(fp); return -1; }

			for ( y = 0; y < SCREEN_HEIGHT; ++y ) {
				for ( i = 0, x = 0; x < SCREEN_WIDTH; ++x ) {
					u32 color = 0;
					u8 r = 0, g = 0, b = 0;
					switch ( pixelformat ) {
					case PSP_DISPLAY_PIXEL_FORMAT_8888:
						color = vram32[x + y * bufferwidth];
						r = color & 0xff; 
						g = (color >> 8) & 0xff;
						b = (color >> 16) & 0xff;
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_565:
						color = vram16[x + y * bufferwidth];
						r = (u8)( ((color & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						g = (u8)( (((color >> 5) & 0x3f) << 2) * 255.0 / 0x3F + 0.5 );
						b = (u8)( (((color >> 11) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_5551:
						color = vram16[x + y * bufferwidth];
						r = (u8)( ((color & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						g = (u8)( (((color >> 5) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						b = (u8)( (((color >> 10) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_4444:
						color = vram16[x + y * bufferwidth];
						r = (u8)( ((color & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						g = (u8)( (((color >> 4) & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						b = (u8)( (((color >> 8) & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						break;
					}
					linebuffer[i++] = r;
					linebuffer[i++] = g;
					linebuffer[i++] = b;
				}
				png_write_row(png_ptr, linebuffer);
			}
			free(linebuffer);
			png_write_end(png_ptr, info_ptr);
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			fclose(fp);
			return 0;
		}

#endif		

#ifdef DXP_BUILDOPTION_USE_LIBJPEG

	case DXP_SAVE_TYPE_JPEG:
		{
			int x = 0, y = 0;
			struct jpeg_compress_struct cinfo;
			struct jpeg_error_mgr jerr;
			JSAMPROW row_pointer[1]; 
			cinfo.err = jpeg_std_error(&jerr);
			jpeg_create_compress(&cinfo);
			jpeg_stdio_dest(&cinfo, fp);
			cinfo.image_width  = SCREEN_WIDTH; 
			cinfo.image_height = SCREEN_HEIGHT;
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			jpeg_set_defaults(&cinfo);
			
			//quality
			jpeg_set_quality(&cinfo, dxpjpegsetting.jquality , TRUE );
			if( dxpjpegsetting.progression ) jpeg_simple_progression (&cinfo);

			jpeg_start_compress(&cinfo, TRUE);

			u8 *linebuffer = (u8*)malloc( sizeof(u8) * SCREEN_WIDTH * 3 );
			if ( !linebuffer ) { fclose(fp); return -1; }

			while ( cinfo.next_scanline < cinfo.image_height ) {
				for ( x = 0; x < SCREEN_WIDTH; ++x ) {
					u32 color = 0;
					u8 r = 0, g = 0, b = 0;
					switch (pixelformat) {
					case PSP_DISPLAY_PIXEL_FORMAT_8888:
						color = vram32[x + y * bufferwidth];
						r = color & 0xff; 
						g = (color >> 8) & 0xff;
						b = (color >> 16) & 0xff;
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_565:
						color = vram16[x + y * bufferwidth];
						r = (u8)( ((color & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						g = (u8)( (((color >> 5) & 0x3f) << 2) * 255.0 / 0x3F + 0.5 );
						b = (u8)( (((color >> 11) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_5551:
						color = vram16[x + y * bufferwidth];
						r = (u8)( ((color & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						g = (u8)( (((color >> 5) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						b = (u8)( (((color >> 10) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_4444:
						color = vram16[x + y * bufferwidth];
						r = (u8)( ((color & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						g = (u8)( (((color >> 4) & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						b = (u8)( (((color >> 8) & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						break;
					}
					linebuffer[x * 3 + 0] = r;
					linebuffer[x * 3 + 1] = g;
					linebuffer[x * 3 + 2] = b;
				}

				row_pointer[0] = linebuffer;
				jpeg_write_scanlines(&cinfo, row_pointer, 1);
				++y;
			}
			
			free(linebuffer);
			jpeg_finish_compress(&cinfo);
			jpeg_destroy_compress(&cinfo);

			fclose(fp);
			return 0;
		}
#endif

	case DXP_SAVE_TYPE_BMP:
		{
			int x = 0, y = 0, count = 0;
			char wbuffer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

			//you can use 24bit or 32bit format
			const int SaveBit = 24;

			unsigned int line_size = SCREEN_WIDTH * (SaveBit >> 3);
			if ( (line_size % 4) != 0 ) line_size = ((line_size >> 2) + 1) << 2;
			
			char magic[2] = "BM";
			fwrite(&magic, 1, 2, fp);

			//bitmap header
			DXP_BMP_HEADER bitmap_header = {
				/* file header */
				(line_size * SCREEN_HEIGHT) + BMP_HEADER_SIZE,
				0,
				0,
				BMP_HEADER_SIZE,
				/* info header */
				40,
				(int)SCREEN_WIDTH,
				(int)SCREEN_HEIGHT,
				1,
				SaveBit,
				0,
				SCREEN_WIDTH * SCREEN_HEIGHT * (SaveBit >> 3),
				0xEC4,
				0xEC4,
				0,
				0
			};

			fwrite(&bitmap_header, 1, sizeof(DXP_BMP_HEADER), fp);
	
			for ( y = SCREEN_HEIGHT - 1; y >= 0; --y ) {
				for ( count = 0, x = 0; x < SCREEN_WIDTH; ++x ) {
					u32 color = 0;
					switch ( pixelformat ) {
					case PSP_DISPLAY_PIXEL_FORMAT_8888:
						color = vram32[x + y * bufferwidth];
						wbuffer[2] = color & 0xff; 
						wbuffer[1] = (color >> 8) & 0xff;
						wbuffer[0] = (color >> 16) & 0xff;
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_565:
						color = vram16[x + y * bufferwidth];
						wbuffer[2] = (u8)( ((color & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						wbuffer[1] = (u8)( (((color >> 5) & 0x3f) << 2) * 255.0 / 0x3F + 0.5 );
						wbuffer[0] = (u8)( (((color >> 11) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_5551:
						color = vram16[x + y * bufferwidth];
						wbuffer[2] = (u8)( ((color & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						wbuffer[1] = (u8)( (((color >> 5) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						wbuffer[0] = (u8)( (((color >> 10) & 0x1f) << 3) * 255.0 / 0x1F + 0.5 );
						break;
					case PSP_DISPLAY_PIXEL_FORMAT_4444:
						color = vram16[x + y * bufferwidth];
						wbuffer[2] = (u8)( ((color & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						wbuffer[1] = (u8)( (((color >> 4) & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						wbuffer[0] = (u8)( (((color >> 8) & 0xf) << 4) * 255.0 / 0xF + 0.5 );
						break;
					}

					fwrite(wbuffer, 1, (SaveBit >> 3), fp);
					count += (SaveBit >> 3);
				}

				if ( count != line_size ) {
					while ( count < line_size ) {
						fputc(0, fp); ++count;
					}
				}
			}

			fclose(fp);
			return 0;
		}

	default:
		fclose(fp);
		return -1;
	}
}
