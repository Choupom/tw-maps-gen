#ifndef IMAGE_H
#define IMAGE_H


#include <pnglite.h>



class CImageRead
{
private:
	int m_Width;
	int m_Height;
	unsigned char *m_pPixels;

public:
	bool Open(const char *pFilename);
	unsigned char *GetPixel(int x, int y);
	void GetPixelZoomOut(int x, int y, int ZoomOut, unsigned char *pColor);
	void Close();
};

class CImageWrite
{
private:
	png_t m_Png;
	int m_Width;
	int m_Height;
	unsigned char *m_pPixels;

public:
	bool Open(const char *pFilename, int Width, int Height);
	void SetPixel(int x, int y, unsigned char *pPixel);
	void SetPixels(unsigned char *pData);
	void Save();
};



#endif
