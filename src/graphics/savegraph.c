#include "../graphics.h"

//#include <psptypes.h>
#include <pspdebug.h>
#include <stdio.h>
#include <stdlib.h>

int SaveGraph(int gh, const char *filename, int type)
{	
	GUINITCHECK;
	DXPGRAPHICSHANDLE* gptr;
	GHANDLE2GPTR(gptr,gh);

	if ( gptr->tex->psm != DXP_FMT_8888 ) return -1;
	int swizzleflag = gptr->tex->swizzledflag;
	int alphaflag = gptr->tex->alphabit;
	int destwidth = gptr->tex->umax;
	int destheight = gptr->tex->vmax;
	u8 *raw = (u8*)gptr->tex->texdata;

	sceIoRemove(filename);
	FILE *fp = fopen(filename, "wb");
    if ( !fp ) return -1;

	if ( swizzleflag ) {
		if ( UnswizzleGraph(gptr->handle) < 0 ) {
			fclose(fp);
			return -1;
		}
	}

	switch ( type ) {

#ifdef DXP_BUILDOPTION_USE_LIBPNG

	/* ---------------------------------------------
		Portable Network Graphics
	 --------------------------------------------- */
	case DXP_SAVE_TYPE_PNG:
		{
			int x = 0, y = 0, i = 0;
			png_structp png_ptr;
			png_infop info_ptr;

			png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if ( !png_ptr ) {
				goto err;
			}
			info_ptr = png_create_info_struct(png_ptr);
			if ( !info_ptr ) {
				png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
				goto err;
			}
			png_init_io(png_ptr, fp);
			png_set_IHDR(png_ptr, info_ptr, destwidth, destheight, 8, alphaflag ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
					PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			png_write_info(png_ptr, info_ptr);

			u8* linedata = (u8*)malloc( sizeof(u8) * destwidth * (alphaflag ? 4 : 3) );
			if ( !linedata ) {
				png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
				goto err;
			}

			if ( alphaflag ) {
				for ( y = 0; y < destheight; ++y ) {
					for ( i = 0, x = 0; x < destwidth; ++x ) {
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 0 ];	//R
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 1 ];	//G
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 2 ];	//B
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 3 ];	//A
					}
					png_write_row(png_ptr, linedata);
				}
			} else {
				for ( y = 0; y < destheight; ++y ) {
					for ( i = 0, x = 0; x < destwidth; ++x ) {
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 0 ];	//R
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 1 ];	//G
						linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 2 ];	//B
					}
					png_write_row(png_ptr, linedata);
				}
			}

			free(linedata);
			png_write_end(png_ptr, info_ptr);
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

			break;
		}

#endif
		
#ifdef DXP_BUILDOPTION_USE_LIBJPEG

	/* ---------------------------------------------
		Joint Photographic Experts Group
	 --------------------------------------------- */
	case DXP_SAVE_TYPE_JPEG:
		{
			int x = 0, y = 0,i = 0;
			struct jpeg_compress_struct cinfo;
			struct jpeg_error_mgr jerr;
			JSAMPROW row_pointer[1]; 

			cinfo.err = jpeg_std_error(&jerr);
			jpeg_create_compress(&cinfo);
			jpeg_stdio_dest(&cinfo, fp);

			cinfo.image_width  = destwidth; 
			cinfo.image_height = destheight;
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			jpeg_set_defaults(&cinfo);

			//quality
			jpeg_set_quality( &cinfo, dxpjpegsetting.jquality , TRUE );
			if ( dxpjpegsetting.progression ) jpeg_simple_progression (&cinfo);

			jpeg_start_compress(&cinfo, TRUE);

			//buffer
			u8* linedata = (u8*)malloc( sizeof(u8) * cinfo.image_width * 3 );
			if ( !linedata ) goto err;

			//write
			while (cinfo.next_scanline < cinfo.image_height) {
				for ( i = 0, x = 0; x < cinfo.image_width; ++x ) {
					linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 0 ];
					linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 1 ];
					linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 2 ];
				}
				row_pointer[0] = linedata;
				jpeg_write_scanlines(&cinfo, row_pointer, 1);
				++y;
			}
			
			free(linedata);
			jpeg_finish_compress(&cinfo);
			jpeg_destroy_compress(&cinfo);
			
			break;
		}

#endif
	case DXP_SAVE_TYPE_BMP:
		{
			int x = 0, y = 0, i = 0;

			//you can use 24bit or 32bit format
			const int SaveBit = 24;

			unsigned int line_size = destwidth * (SaveBit >> 3);
			if ( (line_size % 4) != 0 ) line_size = ((line_size >> 2) + 1) << 2;
			
			char magic[2] = "BM";

			//bitmap header
			DXP_BMP_HEADER bitmap_header = {
				/* file header */
				(line_size * destheight) + BMP_HEADER_SIZE,
				0,
				0,
				BMP_HEADER_SIZE,
				/* info header */
				40,
				(int)destwidth,
				(int)destheight,
				1,
				SaveBit,
				0,
				destwidth * destheight * (SaveBit >> 3),
				0xEC4,
				0xEC4,
				0,
				0
			};
			
			fwrite(&magic, 1, 2, fp);
			fwrite(&bitmap_header, 1, sizeof(DXP_BMP_HEADER), fp);
	
#if 1
			u8* linedata = (u8*)malloc( sizeof(u8) * line_size );
			if ( !linedata ) goto err;
			else memset(linedata, 0x0, line_size);

			for ( y = destheight - 1; y >= 0; --y ) {
				for ( i = 0, x = 0; x < destwidth; ++x ) {
					linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 2 ];
					linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 1 ];
					linedata[i++] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 0 ];
					if ( SaveBit == 32 ) {
						linedata[i++] = 0xFF;
					}
				}
				fwrite(linedata, 1, line_size, fp);
			}

			free(linedata);
#else
			char wbuffer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

			for ( y = destheight - 1; y >= 0; --y ) {
				for ( i = 0, x = 0; x < destwidth; ++x ) {
					wbuffer[2] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 2 ];
					wbuffer[1] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 1 ];
					wbuffer[0] = raw[ y * gptr->tex->pitch * 4 + x * 4 + 0 ];

					fwrite(wbuffer, 1, (SaveBit >> 3), fp);
					i += (SaveBit >> 3);
				}

				if ( i != line_size ) {
					while ( i < line_size ) {
						fputc(0, fp); ++i;
					}
				}
			}
#endif
			break;
		}

	default:
		goto err;
	}

	fclose(fp);
	if ( swizzleflag ) SwizzleGraph(gptr->handle);
	return 0;

err:
	fclose(fp);
	if ( swizzleflag ) SwizzleGraph(gptr->handle);
	return -1;

}
