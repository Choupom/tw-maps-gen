#include "mapimages.h"
#include "mapitems.h"



bool CTilemap::Open(const char *pFilename, CTileset *pTileset, int Width, int Height)
{
	m_pTileset = pTileset;
	return CImageWrite::Open(pFilename, Width*64/ZOOM_OUT, Height*64/ZOOM_OUT);
}

void CTilemap::SetTile(int PosX, int PosY, int Index, int Flags)
{
	PosX = PosX*64/ZOOM_OUT;
	PosY = PosY*64/ZOOM_OUT;
	int IndexX = Index%16 * 64;
	int IndexY = Index/16 * 64;
	
	bool VFlip = Flags & TILEFLAG_VFLIP;
	bool HFlip = Flags & TILEFLAG_HFLIP;
	bool Rotate = Flags & TILEFLAG_ROTATE;
	
	int PX;
	int PY;
	unsigned char aColor[4];
	
	for(int PixelX = 0; PixelX < 64/ZOOM_OUT; PixelX++)
	{
		for(int PixelY = 0; PixelY < 64/ZOOM_OUT; PixelY++)
		{
			PX = PixelX*ZOOM_OUT;
			PY = PixelY*ZOOM_OUT;
			if(Rotate)
			{
				int Temp = PY;
				PY = 64-ZOOM_OUT - PX;
				PX = Temp;
			}
			if(VFlip)
				PX = 64-ZOOM_OUT - PX;
			if(HFlip)
				PY = 64-ZOOM_OUT - PY;
			
			m_pTileset->GetPixelZoomOut(IndexX+PX, IndexY+PY, ZOOM_OUT, aColor);
			SetPixel(PosX+PixelX, PosY+PixelY, aColor);
		}
	}
}
