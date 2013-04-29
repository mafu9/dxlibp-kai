#include "../graphics.h"

#if 0
typedef struct __GHDATA {
	void *texdata;
	void *vramdata;
	int pitch;
	int height;
	int width;
	int pixelformat;
	int swizzled;
} GHDATA ;
#endif

int GetGraphInfo(int gh, GHDATA *dest)
{
	DXPGRAPHICSHANDLE *gptr;
	GUINITCHECK;
	GHANDLE2GPTR(gptr,gh);
	if ( dest == NULL ) return -1;
	
	dest->texdata = gptr->tex->texdata;
	dest->vramdata = gptr->tex->texvram;
	dest->pitch = gptr->tex->pitch;
	dest->pixelformat = gptr->tex->psm;
	dest->swizzled = gptr->tex->swizzledflag;
	dest->width = gptr->u1 - gptr->u0;
	dest->height = gptr->v1 - gptr->v0;
	
	return 0;
}