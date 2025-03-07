
#include <iostream>
#include <Windows.h>

#include "IniReader.h"


CIniReader::CIniReader(const char*  szFileName)
{
	memset(m_szFileName, 0x00, 255);
	memcpy(m_szFileName, szFileName, strlen(szFileName));
}
int CIniReader::ReadInteger( const char* szSection, const char* szKey, int iDefaultValue)
{
	int iResult = GetPrivateProfileIntA(szSection, szKey, iDefaultValue, m_szFileName);
	return iResult;
}
float CIniReader::ReadFloat( const char* szSection, const char* szKey, float fltDefaultValue)
{
	char szResult[255];
	char szDefault[255];
	float fltResult;
	sprintf_s(szDefault, 255, "%f", fltDefaultValue);
	GetPrivateProfileStringA(szSection, szKey, szDefault, szResult, 255, m_szFileName);
	fltResult = (float)atof(szResult);
	return fltResult;
}
BOOL CIniReader::ReadBOOLean( const char* szSection, const  char* szKey, BOOL bolDefaultValue)
{
	char szResult[255];
	char szDefault[255];
	BOOL bolResult;
	sprintf_s(szDefault, 255, "%s", bolDefaultValue ? "TRUE" : "FALSE");
	GetPrivateProfileStringA(szSection, szKey, szDefault, szResult, 255, m_szFileName);
	bolResult = (_stricmp(szResult, "TRUE") == 0 ||
		_stricmp(szResult, "TRUE") == 0) ? TRUE : FALSE;
	return bolResult;
}
char* CIniReader::ReadString( const char* szSection, const  char* szKey, const char* szDefaultValue)
{
	char* szResult = new char[255];
	memset(szResult, 0x00, 255);
	GetPrivateProfileStringA(szSection, szKey,
		szDefaultValue, szResult, 255, m_szFileName);
	return szResult;
}