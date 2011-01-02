#ifndef MAPIMAGES_H
#define MAPIMAGES_H


#include "image.h"



enum
{
	ZOOM_OUT=4
};


class CTileset : public CImageRead
{
};


class CTilemap : public CImageWrite
{
private:
	CTileset *m_pTileset;

public:
	bool Open(const char *pFilename, CTileset *pTileset, int Width, int Height);
	void SetTile(int x, int y, int Index, int Flags);
};



#endif
