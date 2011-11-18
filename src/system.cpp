#include <string.h>

#include "system.h"

#if defined(CONF_FAMILY_WINDOWS)
	#include <direct.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
#endif


void MakeDir(const char *pPath)
{
#if defined(CONF_FAMILY_WINDOWS)
	mkdir(pPath);
#else
	mkdir(pPath, 0775);
#endif
}

void RemoveExtension(char *pBuffer, const char *pExtension)
{
	int LastPoint = -1;
	for(int i = 0; pBuffer[i] != 0; i++)
	{
		if(pBuffer[i] == '.')
			LastPoint = i;
	}
	
	if(LastPoint != -1 && strcmp(&pBuffer[LastPoint], pExtension) == 0)
		pBuffer[LastPoint] = 0;
}

void ExtractDir(char *pBuffer)
{
	int Stop = 0;
	for(int i = 0; pBuffer[i] != 0; i++)
	{
		if(pBuffer[i] == '/' || pBuffer[i] == '\\')
			Stop = i+1;
	}
	
	pBuffer[Stop] = 0;
}
