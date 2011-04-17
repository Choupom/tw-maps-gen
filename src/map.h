#ifndef MAP_H
#define MAP_H


#include "datafile.h"



class CMapReader
{
private:
	CDataFileReader m_Reader;
	char m_aMapname[128];

public:
	bool Open(char *pMapname);
	void Generate(char *pEntities, int TileSize);
	void Close();
};



#endif
