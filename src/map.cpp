#include <stdio.h> //sprintf()
#include <string.h> //strcpy(), strlen()
#include <time.h> //time()

#include "system.h"
#include "mapimages.h"
#include "mapitems.h"
#include "map.h"



bool CMapReader::Open(const char *pMapname)
{
	strcpy(m_aMapname, pMapname);
	char aFilename[256];
	sprintf(aFilename, "maps/%s.map", pMapname);
	return m_Reader.Open(aFilename);
}

void CMapReader::Generate(const char *pEntities)
{
	// create folders
	
	char aGeneratedFolder[256];
	sprintf(aGeneratedFolder, "generated/%s", m_aMapname, sizeof(m_aMapname));
	
	char aGeneratedMapresFolder[256];
	sprintf(aGeneratedMapresFolder, "%s/mapres", aGeneratedFolder);
	
	MakeDir(aGeneratedFolder);
	MakeDir(aGeneratedMapresFolder);
	
	
	// check map version
	
	CMapItemVersion *pVersion = (CMapItemVersion *)m_Reader.FindItem(MAPITEMTYPE_VERSION, 0);
	if(!pVersion || pVersion->m_Version != 1)
		return;
	
	
	// create generating file
	
	char aGenerating[256];
	sprintf(aGenerating, "%s/generating", aGeneratedFolder);
	
	std::fstream GeneratingFile(aGenerating, std::fstream::out | std::fstream::binary);
	GeneratingFile.close();
	
	
	// create background file
	
	char aBackground[256];
	sprintf(aBackground, "%s/background", aGeneratedFolder);
	
	std::fstream BackgroundFile(aBackground, std::fstream::out);
	BackgroundFile.write(aDefaultBackground, strlen(aDefaultBackground));
	BackgroundFile.close();
	
	m_BackgroundFound = false;
	
	
	// load images
	
	int ImagesStart;
	int ImagesNum;
	m_Reader.GetType(MAPITEMTYPE_IMAGE, &ImagesStart, &ImagesNum);
	
	CMapItemImage *pImages = NULL;
	
	if(ImagesNum > 0)
	{
		pImages = new CMapItemImage[ImagesNum];
		
		for(int i = ImagesStart; i < ImagesStart+ImagesNum; i++)
		{
			CMapItemImage *pImage = (CMapItemImage *)m_Reader.GetItem(i, NULL, NULL);
			
			if(!pImage->m_External)
			{
				char *pName = (char *)m_Reader.GetData(pImage->m_ImageName);
				unsigned char *pData = (unsigned char *)m_Reader.GetData(pImage->m_ImageData);
				
				char aImageFilename[512];
				sprintf(aImageFilename, "%s/%s.png", aGeneratedMapresFolder, pName);
				
				CImageWrite Image;
				Image.Open(aImageFilename, pImage->m_Width, pImage->m_Height);
				Image.SetPixels(pData);
				Image.Save();
			}
			
			pImages[i-ImagesStart] = *pImage;
		}
	}
	
	
	// load groups and layers
	
	int GroupsStart;
	int GroupsNum;
	m_Reader.GetType(MAPITEMTYPE_GROUP, &GroupsStart, &GroupsNum);
	
	int LayersStart;
	int LayersNum;
	m_Reader.GetType(MAPITEMTYPE_LAYER, &LayersStart, &LayersNum);
	
	for(int g = GroupsStart; g < GroupsStart+GroupsNum; g++)
	{
		CMapItemGroup *pGroup = (CMapItemGroup *)m_Reader.GetItem(g, NULL, NULL);
		
		if(pGroup->m_Version < 1 || pGroup->m_Version > 2)
			continue;
		
		for(int l = LayersStart + pGroup->m_StartLayer; l < LayersStart + pGroup->m_StartLayer+pGroup->m_NumLayers; l++)
		{
			if(l >= LayersStart+LayersNum)
				break;
			
			CMapItemLayer *pLayer = (CMapItemLayer *)m_Reader.GetItem(l, NULL, NULL);
			
			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTilesLayer = (CMapItemLayerTilemap *)pLayer;
				
				bool GameLayer = (pTilesLayer->m_Flags & TILESLAYERFLAG_GAME) ? true : false;
				
				char *pTilesetName;
				if(GameLayer)
					pTilesetName = (char *)pEntities;
				else if(pTilesLayer->m_Image < 0)
					continue;
				else
					pTilesetName = (char *)m_Reader.GetData(pImages[pTilesLayer->m_Image].m_ImageName);
				
				char aTilesetFilename[512];
				if(GameLayer || pImages[pTilesLayer->m_Image].m_External)
					sprintf(aTilesetFilename, "mapres/%s.png", pTilesetName);
				else
					sprintf(aTilesetFilename, "%s/%s.png", aGeneratedMapresFolder, pTilesetName);
				
				CTileset Src;
				bool Success = Src.Open(aTilesetFilename);
				if(!Success)
					continue;
				
				char aTilemapFilename[512];
				sprintf(aTilemapFilename, "%s/%d.png", aGeneratedFolder, pTilesLayer->m_Data);
				
				CTilemap Dest;
				Dest.Open(aTilemapFilename, &Src, pTilesLayer->m_Width, pTilesLayer->m_Height);
				
				CTile *pTilesData = (CTile *)m_Reader.GetData(pTilesLayer->m_Data);
				
				for(int x = 0; x < pTilesLayer->m_Width; x++)
				{
					for(int y = 0; y < pTilesLayer->m_Height; y++)
					{
						CTile *pTile = &pTilesData[y*pTilesLayer->m_Width + x];
						
						if(pTile->m_Index == 0)
							continue;
						
						Dest.SetTile(x, y, pTile->m_Index, pTile->m_Flags);
					}
				}
				
				Dest.Save();
				Src.Close();
			}
			
			else if(pLayer->m_Type == LAYERTYPE_QUADS)
			{
				CMapItemLayerQuads *pQuadsLayer = (CMapItemLayerQuads *)pLayer;
				
				if(pQuadsLayer->m_Image == -1 && !m_BackgroundFound) // && pGroup->m_ParallaxX == 0 && pGroup->m_ParallaxY == 0
				{
					CQuad *pQuadsData = (CQuad *)m_Reader.GetData(pQuadsLayer->m_Data);
					
					for(int q = 0; q < pQuadsLayer->m_NumQuads; q++)
					{
						int Red = (pQuadsData[q].m_aColors[0].r + pQuadsData[q].m_aColors[1].r + pQuadsData[q].m_aColors[2].r + pQuadsData[q].m_aColors[3].r) / 4;
						int Green = (pQuadsData[q].m_aColors[0].g + pQuadsData[q].m_aColors[1].g + pQuadsData[q].m_aColors[2].g + pQuadsData[q].m_aColors[3].g) / 4;
						int Blue = (pQuadsData[q].m_aColors[0].b + pQuadsData[q].m_aColors[1].b + pQuadsData[q].m_aColors[2].b + pQuadsData[q].m_aColors[3].b) / 4;
						int Alpha = (pQuadsData[q].m_aColors[0].a + pQuadsData[q].m_aColors[1].a + pQuadsData[q].m_aColors[2].a + pQuadsData[q].m_aColors[3].a) / 4;
						
						char aBg[16];
						sprintf(aBg, "#%02X%02X%02X", Red, Green, Blue);
						
						BackgroundFile.open(aBackground, std::fstream::out);
						BackgroundFile.write(aBg, strlen(aBg));
						BackgroundFile.close();
						
						m_BackgroundFound = false;
						
						break;
					}
				}
			}
		}
	}
	
	if(ImagesNum > 0)
		delete pImages;
	
	remove(aGenerating);
}


void CMapReader::Close()
{
	m_Reader.Close();
}
