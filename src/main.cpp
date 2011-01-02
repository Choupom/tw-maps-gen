#include <stdlib.h> //EXIT_SUCCESS, EXIT_FAILURE

#include "map.h"



int main(int argc, char *argv[])
{
	if(argc == 3) //[cwd, map, entities]
	{
		CMapReader Reader;
		bool Success = Reader.Open(argv[1]);
		
		if(Success)
		{
			Reader.Generate(argv[2]);
			Reader.Close();
			return EXIT_SUCCESS;
		}
		else
		{
			return EXIT_FAILURE;
		}
	}
	else
	{
		return EXIT_FAILURE;
	}
}
