#include "../fileio.h"

int FileRead_gets(char *buffer,int buffersize,int filehandle)
{
	int i,c;
	if(!dxpFileioData.init)return -1;
	for(i = 0;i < buffersize - 1;++i)
	{
		c = FileRead_getc(filehandle);
		if(c == '\0' || c == '\n' || c == -1)break;
		buffer[i] = c;
	}
	buffer[i] = '\0';
	return i;
}