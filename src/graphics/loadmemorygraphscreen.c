#include "../graphics.h"

int LoadMemoryGraphScreen(int x, int y, const void *data, unsigned int size, int trans)
{
	int gh = LoadMemoryGraph(data, size);
	if ( gh == -1 ) return -1;
	int res = DrawGraph(x, y, gh, trans);
	DeleteGraph(gh);
	return res;
}