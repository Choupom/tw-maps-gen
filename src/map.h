#ifndef MAP_H
#define MAP_H

#include "benchmark.h"
#include "datafile.h"


inline float fx2f(int v) { return v*(1.0f/(1<<10)); }

inline void IntsToStr(const int *pInts, int Num, char *pStr)
{
	while(Num)
	{
		pStr[0] = (((*pInts)>>24)&0xff)-128;
		pStr[1] = (((*pInts)>>16)&0xff)-128;
		pStr[2] = (((*pInts)>>8)&0xff)-128;
		pStr[3] = ((*pInts)&0xff)-128;
		pStr += 4;
		pInts++;
		Num--;
	}
	pStr[-1] = 0;
}


struct CGenInfo
{
	char *m_pCurrentDir;
	char *m_pMap;
	char *m_pEntities;
	int m_TileSize;
	bool m_ShowBenchmark;
	bool m_DumpTilemaps;
	bool m_DumpGameTilemap;
	bool m_DumpQuads;
	bool m_DumpEmbedded;
	bool m_DumpMetadata;
};


class CMapReader
{
private:
	CDataFileReader m_Reader;
	CBenchmark m_ImagesBenchmark;
	CBenchmark m_TilemapsBenchmark;
	CBenchmark m_QuadsBenchmark;
	CBenchmark m_MetadataBenchmark;
	CBenchmark m_OverallBenchmark;

public:
	bool Open(CGenInfo *pInfo);
	void Generate(CGenInfo *pInfo);
	void Close();
};


#endif
