#include <string.h> // strcmp()

#include "system.h"
#include "map.h"



char *pMap;
char *pEntities;
int TileSize;


void PrintHelp()
{
	printf("Use : tw-maps-gen map [-e entities] [-s tilesize]\n");
	printf("Example : tw-maps-gen ctf2 -e entities_race -s 32\n");
}

bool ParseArguments(int argc, char **argv)
{
	// skip cwd
	argc--;
	argv++;
	
	if(argc < 1)
		return false;
	
	pMap = 0;
	pEntities = "entities";
	TileSize = 16;
	
	for(int i = 0; i < argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i], "-e") == 0)
			{
				if(i+1 >= argc)
					return false;
				else
					pEntities = argv[i+1];
				i++;
			}
			else if(strcmp(argv[i], "-s") == 0)
			{
				if(i+1 >= argc)
					return false;
				else
					TileSize = atoi(argv[i+1]);
				i++;
			}
			else
				return false;
		}
		else
		{
			if(pMap)
				return false;
			else
				pMap = argv[i];
		}
	}
	
	if(!pMap)
		return false;
	
	if(TileSize != 1 && TileSize != 2 && TileSize != 4 && TileSize != 8 && TileSize != 16 && TileSize != 32 && TileSize != 64)
		return false;
	
	RemoveExtension(pMap, ".map");
	RemoveExtension(pEntities, ".png");
	
	return true;
}

int main(int argc, char *argv[])
{
	bool Success = ParseArguments(argc, argv);
	if(!Success)
	{
		PrintHelp();
		return 1;
	}
	
	CMapReader Reader;
	Success = Reader.Open(pMap);
	if(!Success)
	{
		printf("Couldn't load map \"%s\"\n", pMap);
		return 1;
	}
	
	Reader.Generate(pEntities, TileSize);
	
	Reader.Close();
	
	return 0;
}
