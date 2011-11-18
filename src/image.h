#ifndef IMAGE_H
#define IMAGE_H

#include <pnglite.h>


class CImageRead
{
protected:
	unsigned char *m_pPixels;
	bool m_FromBuffer;

public:
	int m_Width;
	int m_Height;
	
	bool Open(const char *pFilename);
	bool OpenFromBuffer(unsigned char *pData, int Width, int Height);
	unsigned char *GetPixel(int x, int y);
	void GetPixelScaled(int x, int y, int ScaleX, int ScaleY, unsigned char *pColor);
	void Close();
};

class CImageWrite
{
protected:
	png_t m_Png;
	unsigned char *m_pPixels;

public:
	int m_Width;
	int m_Height;
	
	bool Open(const char *pFilename, int Width, int Height);
	void SetPixel(int x, int y, unsigned char *pPixel);
	void SetPixels(unsigned char *pData);
	void Save();
};


#endif
