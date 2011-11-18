#ifndef XML_H
#define XML_H


enum XML_ATTRIBUTE_TYPES
{
	XML_ATTRIBUTE_TYPE_INT=1,
	XML_ATTRIBUTE_TYPE_BOOL,
	XML_ATTRIBUTE_TYPE_STR
};

struct CXMLAttribute
{
	const char *m_pName;
	int m_Type;
	CXMLAttribute *m_pNextAttribute;

	CXMLAttribute();
	void Save(FILE *pFile);
};

struct CXMLAttributeInt : public CXMLAttribute
{
	int m_Value;
};

struct CXMLAttributeBool : public CXMLAttribute
{
	bool m_Value;
};

struct CXMLAttributeStr : public CXMLAttribute
{
	char *m_pValue;
};


class CXMLItem
{
private:
	const char *m_pName;
	CXMLAttribute *m_pFirstAttribute;
	CXMLItem *m_pFirstItem;

public:
	CXMLItem *m_pNextItem;

	CXMLItem(const char *pName);
	void AddAttribute(CXMLAttribute *pAttribute);
	void AddAttributeInt(const char *pName, int Value);
	void AddAttributeBool(const char *pName, bool Value);
	void AddAttributeStr(const char *pName, char *pValue);
	CXMLItem *AddChild(const char *pName);
	void Save(FILE *pFile, int Tabs);
	void Close();
};


class CXMLDocument
{
private:
	FILE *m_pFile;
	CXMLItem *m_pMainItem;

public:
	CXMLItem *Open(const char *pMainItemName);
	void Save(char *pFilename);
	void Close();
};


#endif
