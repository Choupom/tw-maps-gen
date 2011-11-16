#include <stdio.h> // sprintf()
#include <string.h> // strncpy()
#include <time.h> // time()

#include "system.h"
#include "mapimages.h"
#include "mapitems.h"
#include "map.h"



bool CMapReader::Open(CGenInfo *pInfo)
{
	char aFilename[256];
	sprintf(aFilename, "%smaps/%s.map", pInfo->m_pCurrentDir, pInfo->m_pMap);
	return m_Reader.Open(aFilename);
}

void CMapReader::Generate(CGenInfo *pInfo)
{
	// create folders
	
	char aGeneratedFolder[256];
	sprintf(aGeneratedFolder, "%sgenerated/%s", pInfo->m_pCurrentDir, pInfo->m_pMap);
	
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
	
	FILE *pGeneratingFile = fopen(aGenerating, "wb");
	if(pGeneratingFile)
		fclose(pGeneratingFile);
	
	
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
				
				if((GameLayer && !pInfo->m_DumpGameTilemap) || (!GameLayer && !pInfo->m_DumpTilemaps))
					continue;
				
				if(!GameLayer && pTilesLayer->m_Image < 0)
					continue;
				
				CMapItemImage *pImage = NULL;
				if(!GameLayer)
					pImage = &pImages[pTilesLayer->m_Image];
				
				char *pTilesetName;
				if(GameLayer)
					pTilesetName = pInfo->m_pEntities;
				else
					pTilesetName = (char *)m_Reader.GetData(pImage->m_ImageName);
				
				CTileset Src;
				if(GameLayer || pImage->m_External)
				{
					char aTilesetFilename[512];
					sprintf(aTilesetFilename, "%smapres/%s.png", pInfo->m_pCurrentDir, pTilesetName);
					
					bool Success = Src.Open(aTilesetFilename);
					if(!Success)
						continue;
				}
				else
				{
					bool Success = Src.OpenFromBuffer((unsigned char *)m_Reader.GetData(pImage->m_ImageData), pImage->m_Width, pImage->m_Height);
					if(!Success)
						continue;
				}
				
				char aTilemapFilename[512];
				if(GameLayer)
					sprintf(aTilemapFilename, "%s/tiles_game_%d.png", aGeneratedFolder, pTilesLayer->m_Data);
				else
					sprintf(aTilemapFilename, "%s/tiles_%d.png", aGeneratedFolder, pTilesLayer->m_Data);
				
				CTilemap Dest;
				bool Success = Dest.Open(aTilemapFilename, pTilesLayer->m_Width, pTilesLayer->m_Height, &Src, pInfo->m_TileSize);
				if(!Success)
					continue;
				
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
				
				Dest.Colorize(&pTilesLayer->m_Color);
				
				Dest.Save();
				Src.Close();
			}
			
			else if(pLayer->m_Type == LAYERTYPE_QUADS)
			{
				CMapItemLayerQuads *pQuadsLayer = (CMapItemLayerQuads *)pLayer;
				
				if(!pInfo->m_DumpQuads)
					continue;
				
				CImageRead Src;
				
				CMapItemImage *pImage = NULL;
				if(pQuadsLayer->m_Image >= 0)
					pImage = &pImages[pQuadsLayer->m_Image];
				
				if(pQuadsLayer->m_Image >= 0)
				{
					char *pImageName = (char *)m_Reader.GetData(pImage->m_ImageName);
					
					if(pImage->m_External)
					{
						char aImageFilename[512];
						sprintf(aImageFilename, "%smapres/%s.png", pInfo->m_pCurrentDir, pImageName);
						
						bool Success = Src.Open(aImageFilename);
						if(!Success)
							continue;
					}
					else
					{
						bool Success = Src.OpenFromBuffer((unsigned char *)m_Reader.GetData(pImage->m_ImageData), pImage->m_Width, pImage->m_Height);
						if(!Success)
							continue;
					}
				}
				
				CQuad *pQuadsData = (CQuad *)m_Reader.GetData(pQuadsLayer->m_Data);
				
				for(int q = 0; q < pQuadsLayer->m_NumQuads; q++)
				{
					CQuad *pQuad = &pQuadsData[q];
					
					int MinX = min(min(min(pQuad->m_aPoints[0].x, pQuad->m_aPoints[1].x), pQuad->m_aPoints[2].x), pQuad->m_aPoints[3].x) / 1024;
					int MinY = min(min(min(pQuad->m_aPoints[0].y, pQuad->m_aPoints[1].y), pQuad->m_aPoints[2].y), pQuad->m_aPoints[3].y) / 1024;
					int MaxX = max(max(max(pQuad->m_aPoints[0].x, pQuad->m_aPoints[1].x), pQuad->m_aPoints[2].x), pQuad->m_aPoints[3].x) / 1024;
					int MaxY = max(max(max(pQuad->m_aPoints[0].y, pQuad->m_aPoints[1].y), pQuad->m_aPoints[2].y), pQuad->m_aPoints[3].y) / 1024;
					
					int Width = MaxX-MinX;
					int Height = MaxY-MinY;
					
					if(Width <= 0 || Height <= 0)
						continue;
					
					char aQuadsFilename[512];
					sprintf(aQuadsFilename, "%s/quads_%d_%d.png", aGeneratedFolder, pQuadsLayer->m_Data, q);
					
					CQuads Dest;
					bool Success = Dest.Open(aQuadsFilename, Width, Height, pInfo->m_TileSize);
					if(!Success)
						continue;
					
					if(pQuadsLayer->m_Image < 0)
						Dest.FillWhite();
					else
						Dest.DrawImage(&Src);
					
					Dest.DrawGradient(pQuad->m_aColors);
					
					Dest.Save();
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
