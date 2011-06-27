#include <stdlib.h> // atoi()
#include <string.h> // strcmp()

#include "system.h"
#include "map.h"



void PrintHelp()
{
	printf("Use : tw-maps-gen map [-q] [-e entities] [-s tilesize]\n");
	printf("Example : tw-maps-gen ctf2 -q -e entities_race -s 32\n");
}

bool CheckTileSize(int Size)
{
	return Size == 1 || Size == 2 || Size == 4 || Size == 8 || Size == 16 || Size == 32 || Size == 64;
}

bool ParseArguments(int argc, char **argv, CGenInfo *pInfo)
{
	// skip cwd
	argc--;
	argv++;
	
	if(argc < 1)
		return false;
	
	// set default parameters
	pInfo->m_pMap = 0;
	pInfo->m_pEntities = "entities";
	pInfo->m_TileSize = 16;
	pInfo->m_DumpQuads = false;
	
	for(int i = 0; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i], "-q") == 0)
			{
				pInfo->m_DumpQuads = true;
			}
			else if(strcmp(argv[i], "-e") == 0)
			{
				if(i+1 >= argc)
					return false;
				else
					pInfo->m_pEntities = argv[i+1];
				i++;
			}
			else if(strcmp(argv[i], "-s") == 0)
			{
				if(i+1 >= argc)
					return false;
				else
					pInfo->m_TileSize = atoi(argv[i+1]);
				i++;
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
	Success = Reader.Open(Info.m_pMap);
	if(!Success)
	{
		printf("Couldn't load map \"%s\"\n", Info.m_pMap);
		return 1;
	}
	
	Reader.Generate(&Info);
	
	Reader.Close();
	
	return 0;
}
