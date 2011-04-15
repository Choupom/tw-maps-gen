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
	bool Open(const char *pFilename, CTileset *pTileset, int Width, int Height, int TileSize);
	void SetTile(int x, int y, int Index, int Flags);
};



#endif
