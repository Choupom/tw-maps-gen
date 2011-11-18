#ifndef MAPIMAGES_H
#define MAPIMAGES_H

#include "image.h"


class CTileset : public CImageRead
{
};

class CTilemap : public CImageWrite
{
private:
	CTileset *m_pTileset;
	int m_TileSize;

public:
	bool Open(const char *pFilename, int Width, int Height, CTileset *pTileset, int TileSize);
	void SetTile(int x, int y, int Index, int Flags);
	void Colorize(struct CColor *pColor);
};

class CQuads : public CImageWrite
{
public:
	bool Open(const char *pFilename, int Width, int Height, int TileSize);
	void FillWhite();
	void DrawImage(CImageRead *m_pImage);
	void DrawGradient(struct CColor *pColors);
};


#endif
