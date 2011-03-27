#include "map.h"



int main(int argc, char *argv[])
{
	if(argc != 2 && argc != 3) // cwd, map, (entities)
	{
		printf("Invalid parameters. Example : \"tw-maps-gen ctf2 entities\".\n");
		return 1;
	}
	
	CMapReader Reader;
	bool Success = Reader.Open(argv[1]);
	
	if(Success)
	{
		if(argc == 3)
			Reader.Generate(argv[2]);
		else
			Reader.Generate("entities");
		Reader.Close();
	}
	else
	{
		printf("Can't load map.\n");
		return 1;
	}
	
	return 0;
}
