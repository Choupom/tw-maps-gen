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
