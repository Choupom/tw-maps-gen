#include <stdio.h> // sprintf()
#include <string.h> // strncpy()

#include "system.h"
#include "mapimages.h"
#include "mapitems.h"
#include "xml.h"
#include "map.h"



bool CMapReader::Open(CGenInfo *pInfo)
{
	char aFilename[256];
	sprintf(aFilename, "%smaps/%s.map", pInfo->m_pCurrentDir, pInfo->m_pMap);
	return m_Reader.Open(aFilename);
}

void CMapReader::Generate(CGenInfo *pInfo)
{
	m_Benchmark.m_Overall.Unpause();
	
	
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
	m_Benchmark.m_ImagesDumping.Unpause();
	
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
			
			if(!pImage->m_External && pInfo->m_DumpEmbedded)
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
	
	m_Benchmark.m_ImagesDumping.Pause();
	
	
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
				m_Benchmark.m_TilemapsDumping.Unpause();
				
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
				
				m_Benchmark.m_TilemapsDumping.Pause();
			}
			
			else if(pLayer->m_Type == LAYERTYPE_QUADS)
			{
				m_Benchmark.m_QuadsDumping.Unpause();
				
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
				
				m_Benchmark.m_QuadsDumping.Pause();
			}
		}
	}
	
	if(pInfo->m_DumpMetadata)
	{
		m_Benchmark.m_MetadataDumping.Unpause();
		
		CXMLDocument Doc;
		CXMLItem *pMainItem = Doc.Open("map");
		pMainItem->AddAttributeInt("version", pVersion->m_Version);
		
		for(int i = 0; i < ImagesNum; i++)
		{
			char *pName = (char *)m_Reader.GetData(pImages[i].m_ImageName);

			CXMLItem *pImageItem = pMainItem->AddChild("image");
			pImageItem->AddAttributeInt("version", pImages[i].m_Version);
			pImageItem->AddAttributeInt("width", pImages[i].m_Width);
			pImageItem->AddAttributeInt("height", pImages[i].m_Height);
			pImageItem->AddAttributeBool("external", pImages[i].m_External);
			pImageItem->AddAttributeStr("name", pName);
		}
		
		for(int g = GroupsStart; g < GroupsStart+GroupsNum; g++)
		{
			CMapItemGroup *pGroup = (CMapItemGroup *)m_Reader.GetItem(g, NULL, NULL);
			
			CXMLItem *pGroupItem = pMainItem->AddChild("group");
			pGroupItem->AddAttributeInt("version", pGroup->m_Version);
			pGroupItem->AddAttributeInt("offset_x", pGroup->m_OffsetX);
			pGroupItem->AddAttributeInt("offset_y", pGroup->m_OffsetY);
			pGroupItem->AddAttributeInt("parallax_x", pGroup->m_ParallaxX);
			pGroupItem->AddAttributeInt("parallax_y", pGroup->m_ParallaxY);
			if(pGroup->m_Version >= 2)
			{
				pGroupItem->AddAttributeBool("use_clipping", pGroup->m_UseClipping);
				pGroupItem->AddAttributeInt("clip_x", pGroup->m_ClipX);
				pGroupItem->AddAttributeInt("clip_y", pGroup->m_ClipY);
				pGroupItem->AddAttributeInt("clip_w", pGroup->m_ClipW);
				pGroupItem->AddAttributeInt("clip_h", pGroup->m_ClipH);
			}
			
			for(int l = LayersStart + pGroup->m_StartLayer; l < LayersStart + pGroup->m_StartLayer+pGroup->m_NumLayers; l++)
			{
				if(l >= LayersStart+LayersNum)
					break;
				
				CMapItemLayer *pLayer = (CMapItemLayer *)m_Reader.GetItem(l, NULL, NULL);
				
				CXMLItem *pLayerItem = pGroupItem->AddChild("layer");
				if(pLayer->m_Type == LAYERTYPE_TILES)
					pLayerItem->AddAttributeStr("type", "tiles");
				else if(pLayer->m_Type == LAYERTYPE_QUADS)
					pLayerItem->AddAttributeStr("type", "quads");
				else
					pLayerItem->AddAttributeStr("type", "invalid");
				pLayerItem->AddAttributeBool("detail", pLayer->m_Flags&TILESLAYERFLAG_GAME);
				
				if(pLayer->m_Type == LAYERTYPE_TILES)
				{
					CMapItemLayerTilemap *pTilesLayer = (CMapItemLayerTilemap *)pLayer;
					
					char *pImageName = "";
					if(pTilesLayer->m_Image >= 0 && pTilesLayer->m_Image < ImagesNum)
						pImageName = (char *)m_Reader.GetData(pImages[pTilesLayer->m_Image].m_ImageName);
					
					pLayerItem->AddAttributeInt("version", pTilesLayer->m_Version);
					pLayerItem->AddAttributeInt("width", pTilesLayer->m_Width);
					pLayerItem->AddAttributeInt("height", pTilesLayer->m_Height);
					pLayerItem->AddAttributeBool("game", pTilesLayer->m_Flags&TILESLAYERFLAG_GAME);
					pLayerItem->AddAttributeInt("color_env", pTilesLayer->m_ColorEnv);
					pLayerItem->AddAttributeInt("color_env_offset", pTilesLayer->m_ColorEnvOffset);
					pLayerItem->AddAttributeStr("image", pImageName);
					pLayerItem->AddAttributeInt("data", pTilesLayer->m_Data);
					
					CXMLItem *pColorItem = pLayerItem->AddChild("color");
					pColorItem->AddAttributeInt("r", pTilesLayer->m_Color.r);
					pColorItem->AddAttributeInt("g", pTilesLayer->m_Color.g);
					pColorItem->AddAttributeInt("b", pTilesLayer->m_Color.b);
					pColorItem->AddAttributeInt("a", pTilesLayer->m_Color.a);
				}
				else if(pLayer->m_Type == LAYERTYPE_QUADS)
				{
					CMapItemLayerQuads *pQuadsLayer = (CMapItemLayerQuads *)pLayer;
					
					char *pImageName = "";
					if(pQuadsLayer->m_Image >= 0 && pQuadsLayer->m_Image < ImagesNum)
						pImageName = (char *)m_Reader.GetData(pImages[pQuadsLayer->m_Image].m_ImageName);
					
					pLayerItem->AddAttributeInt("version", pQuadsLayer->m_Version);
					pLayerItem->AddAttributeStr("image", pImageName);
					pLayerItem->AddAttributeInt("data", pQuadsLayer->m_Data);
					
					CQuad *pQuadsData = (CQuad *)m_Reader.GetData(pQuadsLayer->m_Data);
					
					for(int q = 0; q < pQuadsLayer->m_NumQuads; q++)
					{
						CQuad *pQuad = &pQuadsData[q];
						
						CXMLItem *pQuadItem = pLayerItem->AddChild("quad");
						pQuadItem->AddAttributeInt("pos_env", pQuad->m_PosEnv);
						pQuadItem->AddAttributeInt("pos_env_offset", pQuad->m_PosEnvOffset);
						pQuadItem->AddAttributeInt("color_env", pQuad->m_ColorEnv);
						pQuadItem->AddAttributeInt("color_env_offset", pQuad->m_ColorEnvOffset);
						pQuadItem->AddAttributeInt("data", q);
						
						for(int k = 0; k < 5; k++)
						{
							CXMLItem *pPointItem = pQuadItem->AddChild("point");
							pPointItem->AddAttributeInt("x", fx2f(pQuad->m_aPoints[k].x));
							pPointItem->AddAttributeInt("y", fx2f(pQuad->m_aPoints[k].y));
						}
						
						for(int k = 0; k < 4; k++)
						{
							CXMLItem *pColorItem = pQuadItem->AddChild("color");
							pColorItem->AddAttributeInt("r", pQuad->m_aColors[k].r);
							pColorItem->AddAttributeInt("g", pQuad->m_aColors[k].g);
							pColorItem->AddAttributeInt("b", pQuad->m_aColors[k].b);
							pColorItem->AddAttributeInt("a", pQuad->m_aColors[k].a);
						}
						
						for(int k = 0; k < 4; k++)
						{
							CXMLItem *pTexcoordItem = pQuadItem->AddChild("texcoord");
							pTexcoordItem->AddAttributeInt("x", fx2f(pQuad->m_aTexcoords[k].x));
							pTexcoordItem->AddAttributeInt("y", fx2f(pQuad->m_aTexcoords[k].y));
						}
					}
				}
			}
		}
		
		int EnvPointsStart;
		int EnvPointsNum;
		m_Reader.GetType(MAPITEMTYPE_ENVPOINTS, &EnvPointsStart, &EnvPointsNum);
		
		CEnvPoint *pPoints = 0;
		if(EnvPointsNum)
			pPoints = (CEnvPoint *)m_Reader.GetItem(EnvPointsStart, NULL, NULL);
		
		int EnvStart;
		int EnvNum;
		m_Reader.GetType(MAPITEMTYPE_ENVELOPE, &EnvStart, &EnvNum);
		
		for(int e = EnvStart; e < EnvStart+EnvNum; e++)
		{
			CMapItemEnvelope *pEnv = (CMapItemEnvelope *)m_Reader.GetItem(e, NULL, NULL);
			
			CXMLItem *pEnvItem = pMainItem->AddChild("envelope");
			pEnvItem->AddAttributeInt("version", pEnv->m_Version);
			if(pEnv->m_Channels == 3)
				pEnvItem->AddAttributeStr("type", "pos");
			else if(pEnv->m_Channels == 4)
				pEnvItem->AddAttributeStr("type", "color");
			else
				pEnvItem->AddAttributeStr("type", "invalid");
			if(pEnv->m_aName[0] != -1)
			{
				char aEnvName[64];
				IntsToStr(pEnv->m_aName, sizeof(pEnv->m_aName)/sizeof(int), aEnvName);
				pEnvItem->AddAttributeStr("name", aEnvName);
			}
			
			for(int p = pEnv->m_StartPoint; p < pEnv->m_StartPoint+pEnv->m_NumPoints; p++)
			{
				CXMLItem *pEnvPointItem = pEnvItem->AddChild("envpoint");
				pEnvPointItem->AddAttributeInt("time", pPoints[p].m_Time);
				if(p != pEnv->m_StartPoint+pEnv->m_NumPoints -1)
				{
					if(pPoints[p].m_Curvetype == CURVETYPE_STEP)
						pEnvPointItem->AddAttributeStr("curvetype", "step");
					else if(pPoints[p].m_Curvetype == CURVETYPE_LINEAR)
						pEnvPointItem->AddAttributeStr("curvetype", "linear");
					else if(pPoints[p].m_Curvetype == CURVETYPE_SLOW)
						pEnvPointItem->AddAttributeStr("curvetype", "slow");
					else if(pPoints[p].m_Curvetype == CURVETYPE_FAST)
						pEnvPointItem->AddAttributeStr("curvetype", "fast");
					else if(pPoints[p].m_Curvetype == CURVETYPE_SMOOTH)
						pEnvPointItem->AddAttributeStr("curvetype", "smooth");
					else
						pEnvPointItem->AddAttributeStr("curvetype", "invalid");
				}
				if(pEnv->m_Channels == 3)
				{
					pEnvPointItem->AddAttributeInt("x", fx2f(pPoints[p].m_aValues[0]));
					pEnvPointItem->AddAttributeInt("y", fx2f(pPoints[p].m_aValues[1]));
					pEnvPointItem->AddAttributeInt("r", fx2f(pPoints[p].m_aValues[2]));
				}
				else if(pEnv->m_Channels == 4)
				{
					pEnvPointItem->AddAttributeInt("r", fx2f(pPoints[p].m_aValues[0])*255);
					pEnvPointItem->AddAttributeInt("g", fx2f(pPoints[p].m_aValues[1])*255);
					pEnvPointItem->AddAttributeInt("b", fx2f(pPoints[p].m_aValues[2])*255);
					pEnvPointItem->AddAttributeInt("a", fx2f(pPoints[p].m_aValues[3])*255);
				}
			}
		}
		
		char aMetadataFilename[512];
		sprintf(aMetadataFilename, "%s/metadata.xml", aGeneratedFolder);
		Doc.Save(aMetadataFilename);
		Doc.Close();
		
		m_Benchmark.m_MetadataDumping.Pause();
	}
	
	if(ImagesNum > 0)
		delete pImages;
	
	remove(aGenerating);
	
	m_Benchmark.m_Overall.Pause();
	
	if(pInfo->m_ShowBenchmark)
	{
		printf("Benchmark results:\n");
		printf("  Images dumping:\t%dms\n", m_Benchmark.m_ImagesDumping.GetTime());
		printf("  Tilemaps dumping:\t%dms\n", m_Benchmark.m_TilemapsDumping.GetTime());
		printf("  Quads dumping:\t%dms\n", m_Benchmark.m_QuadsDumping.GetTime());
		printf("  Metadata dumping:\t%dms\n", m_Benchmark.m_MetadataDumping.GetTime());
		printf("  Overall:\t\t%dms\n", m_Benchmark.m_Overall.GetTime());
	}
}


void CMapReader::Close()
{
	m_Reader.Close();
}
