#include <stdlib.h> // malloc(), free()
#include <string.h> // memcpy()

#include "image.h"



bool CImageRead::Open(const char *pFilename)
{
	png_init(NULL, NULL);
	
	png_t Png;
	int Error = png_open_file_read(&Png, pFilename);
	if(Error != PNG_NO_ERROR)
		return false;
	
	m_Width = Png.width;
	m_Height = Png.height;
	
	m_pPixels = (unsigned char *)malloc(m_Width*m_Height*4);
	png_get_data(&Png, m_pPixels);
	png_close_file(&Png);
	
	return true;
}

unsigned char *CImageRead::GetPixel(int x, int y)
{
	int Offset = (y*m_Width + x) * 4;
	return &m_pPixels[Offset];
}

void CImageRead::GetPixelZoomOut(int PixelX, int PixelY, int ZoomOut, unsigned char *pColor)
{
	/*unsigned int aTotalColor[4] = {0, 0, 0, 0};
	for(int x = 0; x < ZoomOut; x++)
	{
		for(int y = 0; y < ZoomOut; y++)
		{
			unsigned char *pPixel = GetPixel(PixelX + x, PixelY + y);
			for(int c = 0; c < 4; c++)
				aTotalColor[c] += pPixel[c];
		}
	}
	for(int c = 0; c < 4; c++)
		pColor[c] = aTotalColor[c] / (ZoomOut*ZoomOut);*/
	
	unsigned char *pPixel1 = GetPixel(PixelX + 0, PixelY + 0);
	unsigned char *pPixel2 = GetPixel(PixelX + ZoomOut-1, PixelY + 0);
	unsigned char *pPixel3 = GetPixel(PixelX + 0, PixelY + ZoomOut-1);
	unsigned char *pPixel4 = GetPixel(PixelX + ZoomOut-1, PixelY + ZoomOut-1);
	for(int c = 0; c < 4; c++)
		pColor[c] = (pPixel1[c] + pPixel2[c] + pPixel3[c] + pPixel4[c]) / 4;
}

void CImageRead::Close()
{
	free(m_pPixels);
}



bool CImageWrite::Open(const char *pFilename, int Width, int Height)
{
	m_Width = Width;
	m_Height = Height;
	
	png_init(NULL, NULL);
	
	int Error = png_open_file_write(&m_Png, pFilename);
	if(Error != PNG_NO_ERROR)
		return false;
	
	m_pPixels = (unsigned char *)malloc(m_Width*m_Height*4);
	
	unsigned char aTransparent[4] = {0, 0, 0, 0};
	for(int i = 0; i < m_Width*m_Height; i++)
		memcpy(&m_pPixels[i*4], aTransparent, 4);
	
	return true;
}

void CImageWrite::SetPixel(int x, int y, unsigned char *pPixel)
{
	int Offset = (y*m_Width + x) * 4;
	memcpy(&m_pPixels[Offset], pPixel, 4);
}

void CImageWrite::SetPixels(unsigned char *pData)
{
	memcpy(m_pPixels, pData, m_Width*m_Height*4);
}

void CImageWrite::Save()
{
	png_set_data(&m_Png, m_Width, m_Height, 8, PNG_TRUECOLOR_ALPHA, m_pPixels);
	png_close_file(&m_Png);
	free(m_pPixels);
}
