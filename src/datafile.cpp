#include <fstream>
#include <zlib.h>

#include "datafile.h"



CDataFileReader::CDataFileReader()
{
	m_pDataFile = NULL;
}

bool CDataFileReader::Open(const char *pFilename)
{
	m_pDataFile = new CDatafile();
	
	std::fstream *pFile = &m_pDataFile->m_File;
	pFile->open(pFilename, std::fstream::in | std::fstream::binary);
	if(pFile->fail())
	{
		delete m_pDataFile;
		m_pDataFile = NULL;
		return false;
	}
	
	CDatafileHeader *pHeader = &m_pDataFile->m_Header;
	pFile->read((char*)pHeader, sizeof(CDatafileHeader));
	if(pHeader->m_aId[0] != 'A' || pHeader->m_aId[1] != 'T' || pHeader->m_aId[2] != 'A' || pHeader->m_aId[3] != 'D')
	{
		if(pHeader->m_aId[0] != 'D' || pHeader->m_aId[1] != 'A' || pHeader->m_aId[2] != 'T' || pHeader->m_aId[3] != 'A')
		{
			m_pDataFile->m_File.close();
			delete m_pDataFile;
			m_pDataFile = NULL;
			return false;
		}
	}
	
	if(pHeader->m_Version != 3 && pHeader->m_Version != 4)
	{
		m_pDataFile->m_File.close();
		delete m_pDataFile;
		m_pDataFile = NULL;
		return false;
	}
	
	
	// count info size
	unsigned int InfoSize = 0;
	InfoSize += pHeader->m_NumItemTypes*sizeof(CDatafileItemType);
	InfoSize += (pHeader->m_NumItems+pHeader->m_NumRawData)*sizeof(int);
	if(pHeader->m_Version == 4)
		InfoSize += pHeader->m_NumRawData*sizeof(int); // v4 has uncompressed data sizes aswell
	InfoSize += pHeader->m_ItemSize;
	
	// read info
	m_pDataFile->m_Info.m_pData = (char *)malloc(InfoSize);
	pFile->read(m_pDataFile->m_Info.m_pData, InfoSize);
	if(pFile->eof())
	{
		free(m_pDataFile->m_Info.m_pData);
		m_pDataFile->m_File.close();
		delete m_pDataFile;
		m_pDataFile = NULL;
		return false;
	}
	
	m_pDataFile->m_Info.m_pItemTypes = (CDatafileItemType *)m_pDataFile->m_Info.m_pData;
	m_pDataFile->m_Info.m_pItemOffsets = (int *)&m_pDataFile->m_Info.m_pItemTypes[pHeader->m_NumItemTypes];
	m_pDataFile->m_Info.m_pDataOffsets = (int *)&m_pDataFile->m_Info.m_pItemOffsets[pHeader->m_NumItems];
	m_pDataFile->m_Info.m_pDataSizes = (int *)&m_pDataFile->m_Info.m_pDataOffsets[pHeader->m_NumRawData];
	
	if(pHeader->m_Version == 4)
		m_pDataFile->m_Info.m_pItemStart = (char *)&m_pDataFile->m_Info.m_pDataSizes[pHeader->m_NumRawData];
	else
		m_pDataFile->m_Info.m_pItemStart = (char *)&m_pDataFile->m_Info.m_pDataOffsets[pHeader->m_NumRawData];
	
	
	m_pDataFile->m_pData = new CDatafileData[pHeader->m_NumRawData];
	m_pDataFile->m_DataStartOffset = sizeof(CDatafileHeader) + InfoSize;
	
	
	return true;
}

int CDataFileReader::NumData()
{
	return m_pDataFile->m_Header.m_NumRawData;
}

int CDataFileReader::GetDataSize(int Index)
{
	if(Index == m_pDataFile->m_Header.m_NumRawData-1)
		return m_pDataFile->m_Header.m_DataSize-m_pDataFile->m_Info.m_pDataOffsets[Index];
	else
		return m_pDataFile->m_Info.m_pDataOffsets[Index+1]-m_pDataFile->m_Info.m_pDataOffsets[Index];
}

void *CDataFileReader::GetData(int Index)
{
	// load it if needed
	if(!m_pDataFile->m_pData[Index].m_Loaded)
	{
		// fetch the data size
		int DataSize = GetDataSize(Index);
		int SwapSize = DataSize;
		
		if(m_pDataFile->m_Header.m_Version == 4)
		{
			// v4 has compressed data
			char *pCompressedData = (char *)malloc(DataSize);
			unsigned long int UncompressedSize = m_pDataFile->m_Info.m_pDataSizes[Index];
			unsigned long int s = UncompressedSize;
			
			m_pDataFile->m_pData[Index].m_pData = (char *)malloc(UncompressedSize);
			
			// read the compressed data
			m_pDataFile->m_File.seekg(m_pDataFile->m_DataStartOffset+m_pDataFile->m_Info.m_pDataOffsets[Index], std::ios_base::beg);
			m_pDataFile->m_File.read(pCompressedData, DataSize);
			
			// decompress the data, TODO: check for errors
			uncompress((Bytef *)m_pDataFile->m_pData[Index].m_pData, &s, (Bytef *)pCompressedData, DataSize); // ignore_convention
			
			// clean up the temporary buffers
			free(pCompressedData);
		}
		else
		{
			// load the data
			m_pDataFile->m_pData[Index].m_pData = (char *)malloc(DataSize);
			m_pDataFile->m_File.seekg(m_pDataFile->m_DataStartOffset+m_pDataFile->m_Info.m_pDataOffsets[Index], std::ios_base::beg);
			m_pDataFile->m_File.read(m_pDataFile->m_pData[Index].m_pData, DataSize);
		}
		
		m_pDataFile->m_pData[Index].m_Loaded = true;
	}
	
	return m_pDataFile->m_pData[Index].m_pData;
}

int CDataFileReader::GetItemSize(int Index)
{
	if(Index == m_pDataFile->m_Header.m_NumItems-1)
		return m_pDataFile->m_Header.m_ItemSize-m_pDataFile->m_Info.m_pItemOffsets[Index];
	else
		return m_pDataFile->m_Info.m_pItemOffsets[Index+1]-m_pDataFile->m_Info.m_pItemOffsets[Index];
}

void *CDataFileReader::GetItem(int Index, int *pType, int *pId)
{
	CDatafileItem *i = (CDatafileItem *)(m_pDataFile->m_Info.m_pItemStart+m_pDataFile->m_Info.m_pItemOffsets[Index]);
	if(pType)
		*pType = (i->m_TypeAndId>>16)&0xffff; // remove sign extention
	if(pId)
		*pId = i->m_TypeAndId&0xffff;
	return (void *)(i+1);
}

void CDataFileReader::GetType(int Type, int *pStart, int *pNum)
{
	*pStart = 0;
	*pNum = 0;
	
	for(int i = 0; i < m_pDataFile->m_Header.m_NumItemTypes; i++)
	{
		if(m_pDataFile->m_Info.m_pItemTypes[i].m_Type == Type)
		{
			*pStart = m_pDataFile->m_Info.m_pItemTypes[i].m_Start;
			*pNum = m_pDataFile->m_Info.m_pItemTypes[i].m_Num;
			break;
		}
	}
}

void *CDataFileReader::FindItem(int Type, int Id)
{
	int Start, Num;
	GetType(Type, &Start, &Num);
	for(int i = 0; i < Num; i++)
	{
		int ItemId;
		void *pItem = GetItem(Start+i,0, &ItemId);
		if(Id == ItemId)
			return pItem;
	}
	return 0;
}

int CDataFileReader::NumItems()
{
	return m_pDataFile->m_Header.m_NumItems;
}

void CDataFileReader::UnloadData(int Index)
{
	if(m_pDataFile->m_pData[Index].m_Loaded)
	{
		free(m_pDataFile->m_pData[Index].m_pData);
		m_pDataFile->m_pData[Index].m_pData = NULL;
	}
}

void CDataFileReader::Close()
{
	if(!m_pDataFile)
		return;
	
	free(m_pDataFile->m_Info.m_pData);
	
	for(int i = 0; i < m_pDataFile->m_Header.m_NumRawData; i++)
		UnloadData(i);
	
	delete m_pDataFile->m_pData;
	
	m_pDataFile->m_File.close();
	delete m_pDataFile;
	m_pDataFile = NULL;
}
