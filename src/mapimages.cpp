#include <string.h> // memset()

#include "system.h"
#include "mapimages.h"
#include "mapitems.h"



bool CTilemap::Open(const char *pFilename, int Width, int Height, CTileset *pTileset, int TileSize)
{
	m_pTileset = pTileset;
	m_TileSize = TileSize;
	return CImageWrite::Open(pFilename, Width*TileSize, Height*TileSize);
}

void CTilemap::SetTile(int PosX, int PosY, int Index, int Flags)
{
	PosX *= m_TileSize;
	PosY *= m_TileSize;
	int IndexX = Index%16 * 64;
	int IndexY = Index/16 * 64;
	
	bool VFlip = Flags & TILEFLAG_VFLIP;
	bool HFlip = Flags & TILEFLAG_HFLIP;
	bool Rotate = Flags & TILEFLAG_ROTATE;
	
	int Scale = 64/m_TileSize;
	int PX, PY;
	unsigned char aColor[4];
	
	for(int PixelX = 0; PixelX < m_TileSize; PixelX++)
	{
		for(int PixelY = 0; PixelY < m_TileSize; PixelY++)
		{
			PX = PixelX*Scale;
			PY = PixelY*Scale;
			if(Rotate)
			{
				int Temp = PY;
				PY = 64-Scale - PX;
				PX = Temp;
			}
			if(VFlip)
				PX = 64-Scale - PX;
			if(HFlip)
				PY = 64-Scale - PY;
			
			m_pTileset->GetPixelScaled(IndexX+PX, IndexY+PY, Scale, aColor);
			SetPixel(PosX+PixelX, PosY+PixelY, aColor);
		}
	}
}

void CTilemap::Colorize(CColor *pColor)
{
	unsigned char aColor[4];
	unsigned char *pPixel;
	
	for(int x = 0; x < m_Width; x++)
	{
		for(int y = 0; y < m_Height; y++)
		{
			pPixel = &m_pPixels[(y*m_Width+x) * 4];
			aColor[0] = pPixel[0] * pColor->r/255.0f;
			aColor[1] = pPixel[1] * pColor->g/255.0f;
			aColor[2] = pPixel[2] * pColor->b/255.0f;
			aColor[3] = pPixel[3] * pColor->a/255.0f;
			SetPixel(x, y, aColor);
		}
	}
}



bool CQuads::Open(const char *pFilename, int Width, int Height, int TileSize)
{
	return CImageWrite::Open(pFilename, Width*(TileSize/64.0f), Height*(TileSize/64.0f));
}

void CQuads::FillWhite()
{
	memset(m_pPixels, 255, m_Width*m_Height*4);
}

void CQuads::DrawImage(CImageRead *m_pImage)
{
	float ScaleX = (float)m_pImage->m_Width / m_Width;
	float ScaleY = (float)m_pImage->m_Height / m_Height;
	unsigned char aColor[4];
	
	for(int PixelX = 0; PixelX < m_Width; PixelX++)
	{
		for(int PixelY = 0; PixelY < m_Height; PixelY++)
		{
			m_pImage->GetPixelScaled(PixelX*ScaleX, PixelY*ScaleY, min(ScaleX, ScaleY), aColor);
			SetPixel(PixelX, PixelY, aColor);
		}
	}
}

void CQuads::DrawGradient(CColor *pColors)
{
	float fx, fy;
	unsigned char Red, Green, Blue, Alpha;
	unsigned char aColor[4];
	unsigned char *pPixel;
	
	for(int x = 0; x < m_Width; x++)
	{
		for(int y = 0; y < m_Height; y++)
		{
			pPixel = &m_pPixels[(y*m_Width+x) * 4];
			fx = (float)x;
			fy = (float)y;
			Red = (pColors[2].r*fy/m_Height + pColors[0].r*(1-fy/m_Height)) * (fx/m_Width) + (pColors[3].r*fy/m_Height + pColors[1].r*(1-fy/m_Height)) * (1-fx/m_Width);
			Green = (pColors[2].g*fy/m_Height + pColors[0].g*(1-fy/m_Height)) * (fx/m_Width) + (pColors[3].g*fy/m_Height + pColors[1].g*(1-fy/m_Height)) * (1-fx/m_Width);
			Blue = (pColors[2].b*fy/m_Height + pColors[0].b*(1-fy/m_Height)) * (fx/m_Width) + (pColors[3].b*fy/m_Height + pColors[1].b*(1-fy/m_Height)) * (1-fx/m_Width);
			Alpha = (pColors[2].a*fy/m_Height + pColors[0].a*(1-fy/m_Height)) * (fx/m_Width) + (pColors[3].a*fy/m_Height + pColors[1].a*(1-fy/m_Height)) * (1-fx/m_Width);
			aColor[0] = pPixel[0] * Red/255.0f;
			aColor[1] = pPixel[1] * Green/255.0f;
			aColor[2] = pPixel[2] * Blue/255.0f;
			aColor[3] = pPixel[3] * Alpha/255.0f;
			SetPixel(x, y, aColor);
		}
	}
}
