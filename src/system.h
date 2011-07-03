#ifndef SYSTEM_H
#define SYSTEM_H


#if defined(WIN64) || defined(_WIN64) || defined(WIN32) || defined(_WIN32) || defined(__CYGWIN32__) || defined(__MINGW32__)
	#define CONF_FAMILY_WINDOWS 1
#endif


void MakeDir(const char *pPath);
void RemoveExtension(char *pBuffer, const char *pExtension);
void ExtractDir(char *pBuffer);

template <typename T> inline T min(T a, T b) { return a<b?a:b; }
template <typename T> inline T max(T a, T b) { return a>b?a:b; }


#endif
