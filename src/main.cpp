#include <string.h> // strcmp()
#include "map.h"



const char *pMap;
const char *pEntities;


void PrintHelp()
{
	printf("Use : tw-maps-gen map [-e entities]\n");
	printf("Example : tw-maps-gen ctf2 -e entities_race\n");
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
	
	Reader.Generate(pEntities);
	
	Reader.Close();
	
	return 0;
}
