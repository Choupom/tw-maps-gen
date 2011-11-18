#include <stdlib.h> // atoi()
#include <stdio.h> // printf()
#include <string.h> // strcmp()

#include "system.h"
#include "map.h"



void PrintHelp()
{
	printf("Use : tw-maps-gen map [-d tgqem] [-e entities] [-s tilesize] [-b]\n");
	printf("Example : tw-maps-gen ctf2 -d tg -e entities_race -s 32 -b\n");
}

bool CheckTileSize(int Size)
{
	return Size == 1 || Size == 2 || Size == 4 || Size == 8 || Size == 16 || Size == 32 || Size == 64;
}

bool ParseArguments(int argc, char **argv, CGenInfo *pInfo)
{
	if(argc < 2)
		return false;
	
	pInfo->m_pCurrentDir = argv[0];
	ExtractDir(pInfo->m_pCurrentDir);
	pInfo->m_pMap = 0;
	
	// set default parameters
	pInfo->m_pEntities = (char *)"entities";
	pInfo->m_TileSize = 16;
	pInfo->m_ShowBenchmark = false;
	pInfo->m_DumpTilemaps = true;
	pInfo->m_DumpGameTilemap = true;
	pInfo->m_DumpQuads = true;
	pInfo->m_DumpEmbedded = true;
	pInfo->m_DumpMetadata = true;
	
	for(int i = 1; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i], "-b") == 0)
			{
				pInfo->m_ShowBenchmark = true;
			}
			else if(strcmp(argv[i], "-d") == 0)
			{
				i++;
				if(i >= argc)
					return false;
				else
				{
					pInfo->m_DumpTilemaps = false;
					pInfo->m_DumpGameTilemap = false;
					pInfo->m_DumpQuads = false;
					pInfo->m_DumpEmbedded = false;
					pInfo->m_DumpMetadata = false;
					for(int j = 0; j < strlen(argv[i]); j++)
					{
						if(argv[i][j] == 't')
							pInfo->m_DumpTilemaps = true;
						else if(argv[i][j] == 'g')
							pInfo->m_DumpGameTilemap = true;
						else if(argv[i][j] == 'q')
							pInfo->m_DumpQuads = true;
						else if(argv[i][j] == 'e')
							pInfo->m_DumpEmbedded = true;
						else if(argv[i][j] == 'm')
							pInfo->m_DumpMetadata = true;
					}
				}
			}
			else if(strcmp(argv[i], "-e") == 0)
			{
				i++;
				if(i >= argc)
					return false;
				else
					pInfo->m_pEntities = argv[i];
			}
			else if(strcmp(argv[i], "-s") == 0)
			{
				i++;
				if(i >= argc)
					return false;
				else
					pInfo->m_TileSize = atoi(argv[i]);
			}
			else
				return false;
		}
		else
		{
			if(pInfo->m_pMap)
				return false;
			else
				pInfo->m_pMap = argv[i];
		}
	}
	
	if(!pInfo->m_pMap)
		return false;
	
	if(!CheckTileSize(pInfo->m_TileSize))
		return false;
	
	RemoveExtension(pInfo->m_pMap, ".map");
	RemoveExtension(pInfo->m_pEntities, ".png");
	
	return true;
}

int main(int argc, char *argv[])
{
	CGenInfo Info;
	bool Success = ParseArguments(argc, argv, &Info);
	if(!Success)
	{
		PrintHelp();
		return 1;
	}
	
	CMapReader Reader;
	Success = Reader.Open(&Info);
	if(!Success)
	{
		printf("Couldn't load map \"%s\" from \"%smaps/\"\n", Info.m_pMap, Info.m_pCurrentDir);
		return 1;
	}
	
	Reader.Generate(&Info);
	
	Reader.Close();
	
	return 0;
}
