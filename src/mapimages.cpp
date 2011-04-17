#include "mapimages.h"
#include "mapitems.h"



bool CTilemap::Open(const char *pFilename, CTileset *pTileset, int Width, int Height, int TileSize)
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
	
	int ZoomOut = 64/m_TileSize;
	int PX;
	int PY;
	unsigned char aColor[4];
	
	for(int PixelX = 0; PixelX < m_TileSize; PixelX++)
	{
		for(int PixelY = 0; PixelY < m_TileSize; PixelY++)
		{
			PX = PixelX*ZoomOut;
			PY = PixelY*ZoomOut;
			if(Rotate)
			{
				int Temp = PY;
				PY = 64-ZoomOut - PX;
				PX = Temp;
			}
			if(VFlip)
				PX = 64-ZoomOut - PX;
			if(HFlip)
				PY = 64-ZoomOut - PY;
			
			m_pTileset->GetPixelZoomOut(IndexX+PX, IndexY+PY, ZoomOut, aColor);
			SetPixel(PosX+PixelX, PosY+PixelY, aColor);
		}
	}
}



void CQuads::DrawGradient(CColor *pColors)
{
	unsigned char aColor[4];
	for(float x = 0; x < m_Width; x++)
	{
		for(float y = 0; y < m_Height; y++)
		{
			aColor[0] = (pColors[2].r*y/m_Height + pColors[0].r*(1-y/m_Height)) * (x/m_Width) + (pColors[3].r*y/m_Height + pColors[1].r*(1-y/m_Height)) * (1-x/m_Width);
			aColor[1] = (pColors[2].g*y/m_Height + pColors[0].g*(1-y/m_Height)) * (x/m_Width) + (pColors[3].g*y/m_Height + pColors[1].g*(1-y/m_Height)) * (1-x/m_Width);
			aColor[2] = (pColors[2].b*y/m_Height + pColors[0].b*(1-y/m_Height)) * (x/m_Width) + (pColors[3].b*y/m_Height + pColors[1].b*(1-y/m_Height)) * (1-x/m_Width);
			aColor[3] = (pColors[2].a*y/m_Height + pColors[0].a*(1-y/m_Height)) * (x/m_Width) + (pColors[3].a*y/m_Height + pColors[1].a*(1-y/m_Height)) * (1-x/m_Width);
			// TODO: fix alpha
			SetPixel(x, y, aColor);
		}
	}
}
