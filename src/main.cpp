#include <stdlib.h> // atoi()
#include <stdio.h> // printf()
#include <string.h> // strcmp()

#include "version.h"
#include "system.h"
#include "map.h"


void PrintVersion()
{
	printf("tw-maps-gen version: %s\n", TWMAPSGEN_VERSION);
	printf("Supported Teeworlds maps: from %s to %s\n", TWMAPSGEN_SUPPORT_OLDEST, TWMAPSGEN_SUPPORT_NEWEST);
}

void PrintHelp()
{
	printf("Use: tw-maps-gen map [-d tgqem] [-e entities] [-s tilesize] [-b]\n");
	printf("Example: tw-maps-gen ctf2 -d tg -e entities_race -s 32 -b\n");
}

bool CheckTileSize(int Size)
{
	return Size == 1 || Size == 2 || Size == 4 || Size == 8 || Size == 16 || Size == 32 || Size == 64;
}

bool ParseArguments(int argc, char **argv, CGenInfo *pInfo, bool *pShowVersion)
{
	if(argc < 2)
		return false;
	
	*pShowVersion = false;
	
	sprintf(pInfo->m_aCurrentDir, "%s", argv[0]);
	ExtractDir(pInfo->m_aCurrentDir);
	pInfo->m_aMap[0] = 0;
	
	// set default parameters
	sprintf(pInfo->m_aEntities, "entities");
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
					sprintf(pInfo->m_aEntities, "%s", argv[i]);
			}
			else if(strcmp(argv[i], "-s") == 0)
			{
				i++;
				if(i >= argc)
					return false;
				else
					pInfo->m_TileSize = atoi(argv[i]);
			}
			else if(strcmp(argv[i], "-v") == 0)
			{
				*pShowVersion = true;
				return true;
			}
			else
				return false;
		}
		else
		{
			if(pInfo->m_aMap[0])
				return false;
			else
				sprintf(pInfo->m_aMap, "%s", argv[i]);
		}
	}
	
	if(!pInfo->m_aMap[0])
		return false;
	
	if(!CheckTileSize(pInfo->m_TileSize))
		return false;
	
	RemoveExtension(pInfo->m_aMap, ".map");
	RemoveExtension(pInfo->m_aEntities, ".png");
	
	return true;
}

int main(int argc, char *argv[])
{
	bool ShowVersion;
	CGenInfo Info;
	bool Success = ParseArguments(argc, argv, &Info, &ShowVersion);
	
	if(!Success)
	{
		PrintHelp();
		return 1;
	}
	
	if(ShowVersion)
	{
		PrintVersion();
		return 0;
	}
	
	CMapReader Reader;
	Success = Reader.Open(&Info);
	if(!Success)
	{
		printf("Couldn't load map \"%s\" from \"%smaps/\"\n", Info.m_aMap, Info.m_aCurrentDir);
		return 1;
	}
	
	Reader.Generate(&Info);
	
	Reader.Close();
	
	return 0;
}
