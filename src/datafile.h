#ifndef DATAFILE_H
#define DATAFILE_H

#include <stdio.h>


struct CDatafileItemType
{
	int m_Type;
	int m_Start;
	int m_Num;
};

struct CDatafileItem
{
	int m_TypeAndId;
	int m_Size;
};

struct CDatafileHeader
{
	char m_aId[4];
	int m_Version;
	int m_Size;
	int m_Swaplen;
	int m_NumItemTypes;
	int m_NumItems;
	int m_NumRawData;
	int m_ItemSize;
	int m_DataSize;
};

struct CDatafileInfo
{
	char *m_pData;
	CDatafileItemType *m_pItemTypes;
	int *m_pItemOffsets;
	int *m_pDataOffsets;
	int *m_pDataSizes;
	char *m_pItemStart;
};

struct CDatafileData
{
	CDatafileData() { m_Loaded = false; }
	bool m_Loaded;
	char *m_pData;
};

struct CDatafile
{
	FILE *m_pFile;
	CDatafileHeader m_Header;
	CDatafileInfo m_Info;
	CDatafileData *m_pData;
	unsigned int m_DataStartOffset;
};


class CDataFileReader
{
private:
	CDatafile *m_pDataFile;

public:
	CDataFileReader();
	
	bool Open(const char *pFilename);
	void Close();
	
	void *GetData(int Index);
	int GetDataSize(int Index);
	void UnloadData(int Index);
	void *GetItem(int Index, int *pType, int *pId);
	int GetItemSize(int Index);
	void GetType(int Type, int *pStart, int *pNum);
	void *FindItem(int Type, int Id);
	int NumItems();
	int NumData();
};


#endif
