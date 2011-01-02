#ifndef SYSTEM_H
#define SYSTEM_H


#if defined(WIN64) || defined(_WIN64) || defined(WIN32) || defined(_WIN32) || defined(__CYGWIN32__) || defined(__MINGW32__)
	#define CONF_FAMILY_WINDOWS 1
#endif

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



#endif
