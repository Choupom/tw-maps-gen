#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xml.h"



CXMLAttribute::CXMLAttribute()
{
	m_pNextAttribute = 0;
}

void CXMLAttribute::Save(FILE *pFile)
{
	fwrite(m_pName, strlen(m_pName), 1, pFile);
	fwrite("=", 1, 1, pFile);
	fwrite("\"", 1, 1, pFile);

	char aBuf[256];

	if(m_Type == XML_ATTRIBUTE_TYPE_INT)
	{
		itoa(((CXMLAttributeInt *)this)->m_Value, aBuf, 10);
	}
	else if(m_Type == XML_ATTRIBUTE_TYPE_BOOL)
	{
		if(((CXMLAttributeBool *)this)->m_Value)
			sprintf(aBuf, "true");
		else
			sprintf(aBuf, "false");
	}
	else if(m_Type == XML_ATTRIBUTE_TYPE_STR)
	{
		sprintf(aBuf, ((CXMLAttributeStr *)this)->m_pValue);
	}

	fwrite(aBuf, strlen(aBuf), 1, pFile);
	fwrite("\"", 1, 1, pFile);
}



CXMLItem::CXMLItem(const char *pName)
{
	m_pName = pName;
	m_pFirstAttribute = 0;
	m_pFirstItem = 0;
	m_pNextItem = 0;
}

void CXMLItem::AddAttribute(CXMLAttribute *pAttribute)
{
	if(m_pFirstAttribute)
	{
		CXMLAttribute *pLastAttribute = m_pFirstAttribute;
		while(pLastAttribute->m_pNextAttribute)
			pLastAttribute = pLastAttribute->m_pNextAttribute;
		pLastAttribute->m_pNextAttribute = (CXMLAttribute *)pAttribute;
	}
	else
		m_pFirstAttribute = pAttribute;
}

void CXMLItem::AddAttributeInt(const char *pName, int Value)
{
	CXMLAttributeInt *pAttribute = new CXMLAttributeInt;
	pAttribute->m_pName = pName;
	pAttribute->m_Type = XML_ATTRIBUTE_TYPE_INT;
	pAttribute->m_pNextAttribute = 0;
	pAttribute->m_Value = Value;
	AddAttribute(pAttribute);
}

void CXMLItem::AddAttributeBool(const char *pName, bool Value)
{
	CXMLAttributeBool *pAttribute = new CXMLAttributeBool;
	pAttribute->m_pName = pName;
	pAttribute->m_Type = XML_ATTRIBUTE_TYPE_BOOL;
	pAttribute->m_pNextAttribute = 0;
	pAttribute->m_Value = Value;
	AddAttribute(pAttribute);
}

void CXMLItem::AddAttributeStr(const char *pName, char *pValue)
{
	CXMLAttributeStr *pAttribute = new CXMLAttributeStr;
	pAttribute->m_pName = pName;
	pAttribute->m_Type = XML_ATTRIBUTE_TYPE_STR;
	pAttribute->m_pNextAttribute = 0;
	pAttribute->m_pValue = pValue;
	AddAttribute(pAttribute);
}

CXMLItem *CXMLItem::AddChild(const char *pName)
{
	CXMLItem *pItem = new CXMLItem(pName);
	if(m_pFirstItem)
	{
		CXMLItem *pLastItem = m_pFirstItem;
		while(pLastItem->m_pNextItem)
			pLastItem = pLastItem->m_pNextItem;
		pLastItem->m_pNextItem = pItem;
	}
	else
		m_pFirstItem = pItem;
	return pItem;
}

void CXMLItem::Save(FILE *pFile, int Tabs)
{
	for(int i = 0; i < Tabs; i++)
		fwrite("\t", 1, 1, pFile);
	fwrite("<", 1, 1, pFile);
	fwrite(m_pName, strlen(m_pName), 1, pFile);

	CXMLAttribute *pAttribute = m_pFirstAttribute;
	while(pAttribute)
	{
		fwrite(" ", 1, 1, pFile);
		pAttribute->Save(pFile);
		pAttribute = pAttribute->m_pNextAttribute;
	}

	if(!m_pFirstItem)
	{
		fwrite("/>", 2, 1, pFile);
		fwrite("\n", 1, 1, pFile);
	}
	else
	{
		fwrite(">", 1, 1, pFile);
		fwrite("\n", 1, 1, pFile);

		CXMLItem *pItem = m_pFirstItem;
		while(pItem)
		{
			pItem->Save(pFile, Tabs+1);
			pItem = pItem->m_pNextItem;
		}

		for(int i = 0; i < Tabs; i++)
			fwrite("\t", 1, 1, pFile);
		fwrite("</", 2, 1, pFile);
		fwrite(m_pName, strlen(m_pName), 1, pFile);
		fwrite(">", 1, 1, pFile);
		fwrite("\n", 1, 1, pFile);
	}
}

void CXMLItem::Close()
{
	CXMLAttribute *pAttribute = m_pFirstAttribute;
	CXMLAttribute *pNextAttribute;
	while(pAttribute)
	{
		pNextAttribute = pAttribute->m_pNextAttribute;
		delete pAttribute;
		pAttribute = pNextAttribute;
	}

	CXMLItem *pItem = m_pFirstItem;
	CXMLItem *pNextItem;
	while(pItem)
	{
		pNextItem = pItem->m_pNextItem;
		delete pItem;
		pItem = pNextItem;
	}
}



CXMLItem *CXMLDocument::Open(const char *pMainItemName)
{
	m_pMainItem = new CXMLItem(pMainItemName);
	return m_pMainItem;
}

void CXMLDocument::Save(char *pFilename)
{
	m_pFile = fopen(pFilename, "w");

	static const char aDeclaration[] = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
	fwrite(aDeclaration, strlen(aDeclaration), 1, m_pFile);

	m_pMainItem->Save(m_pFile, 0);

	fclose(m_pFile);
}

void CXMLDocument::Close()
{
	m_pMainItem->Close();
	delete m_pMainItem;
}
