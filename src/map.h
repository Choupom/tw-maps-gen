#ifndef MAP_H
#define MAP_H


#include "datafile.h"



struct CGenInfo
{
	char *m_pCurrentDir;
	char *m_pMap;
	char *m_pEntities;
	int m_TileSize;
	bool m_DumpQuads;
};


class CMapReader
{
private:
	CDataFileReader m_Reader;

public:
	bool Open(CGenInfo *pInfo);
	void Generate(CGenInfo *pInfo);
	void Close();
};



#endif
