#include "../graphics.h"
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <stdlib.h>

static DXPTEXTURE3* LoadScreen(void);

int MakeGraphFromScreen(void)
{
	GUINITCHECK;
	DXPGRAPHICSHANDLE *gptr = dxpGraphicsCreateGraphicHandle();
	if ( !gptr ) return -1;
	
	DXPTEXTURE3 *texptr = LoadScreen();
	if ( !texptr ) { dxpGraphicsReleseGraphicHandle(gptr); return -1; }
	
	gptr->tex = texptr;
	++texptr->refcount;
	gptr->u0 = gptr->v0 = 0;
	gptr->u1 = texptr->umax;
	gptr->v1 = texptr->vmax;

	if ( dxpGraphicsData.create_vram_graph ) MoveGraphToVRAM( gptr->handle );
	if ( dxpGraphicsData.create_swizzled_graph ) SwizzleGraph( gptr->handle );

	sceKernelDcacheWritebackAll();
	return gptr->handle;
}

static DXPTEXTURE3* LoadScreen(void)
{
	DXPTEXTURE3 *texptr = NULL;

	int y, bufferwidth, pixelformat, pixelsize;
	u8* vram = NULL;

	//display information
	sceDisplayGetFrameBuf((void**)&vram, &bufferwidth, &pixelformat, 0);

	switch ( pixelformat ) {
	case PSP_DISPLAY_PIXEL_FORMAT_8888:
		pixelsize = 4;
		break;
	case PSP_DISPLAY_PIXEL_FORMAT_565:
	case PSP_DISPLAY_PIXEL_FORMAT_5551:
	case PSP_DISPLAY_PIXEL_FORMAT_4444:
		pixelsize = 2;
		break;
	default:
		return NULL;
	}

	//image information
	int width  = SCREEN_WIDTH;
	int height = SCREEN_HEIGHT;
	int pitch = ((width + 3) >> 2) << 2;
	int texwidth = AlignPow2(width);
	int texheight = AlignPow2(height);
	u8 *data = (u8*)malloc(pitch * texheight * pixelsize);
	if ( data == NULL ) {
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	for ( y = 0; y < SCREEN_HEIGHT; y++ )
		memcpy(data + y * pitch * pixelsize, vram + y * bufferwidth * pixelsize, SCREEN_WIDTH * pixelsize);

	texptr = dxpGraphicsCreateTexture();
	if ( !texptr ) {
		free(data);
		dxpGraphicsReleseTexture(texptr);
		return NULL;
	}

	texptr->texdata = data;
	texptr->alphabit = 0;
	texptr->colorkey = dxpGraphicsData.colorkey;
	texptr->width = texwidth;
	texptr->height = texheight;
	texptr->pitch = pitch;
	texptr->ppalette = NULL;
	texptr->psm = pixelformat;
	texptr->reloadflag = 1;
	texptr->size2_nflag = 0;
	texptr->swizzledflag = 0;
	texptr->texvram = 0;
	texptr->umax = width;
	texptr->vmax = height;

	return texptr;
}
