#include "../graphics.h"

int LoadDivMemoryGraph(const void *data, unsigned int size, int allnum, int xnum, int ynum, int xsize, int ysize, int *handlebuf)
{
	if(!handlebuf || !data || !size || allnum <= 0 || xnum <= 0 || ynum <= 0 || xsize <= 0 || ysize <= 0)return -1;
	int gh = LoadMemoryGraph(data, size);
	if(gh == -1)return -1;
	DXPGRAPHICSHANDLE *gptr;
	GHANDLE2GPTR(gptr,gh);
	int i;
	if(xnum * xsize > gptr->tex->width || ynum * ysize > gptr->tex->height)
	{
		DeleteGraph(gh);
		return -1;
	}

	for(i = 0;i < allnum;++i)
	{
		handlebuf[i] = DerivationGraph(xsize * (i % xnum),ysize * (i / xnum),xsize,ysize,gh);
	}
	DeleteGraph(gh);
	return 0;
}