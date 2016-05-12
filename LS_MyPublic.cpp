#include "stdafx.h"
#include "LS_MyPublic.h"

void __cdecl trace_r(const char* _pszFormat, ...)
{
	static const int iMaxPath = 512;
	char pcData[iMaxPath] = {0};
	int iLen = strlen(pcData);
	va_list marker;
	va_start(marker, _pszFormat);
	vsnprintf(pcData+iLen, iMaxPath-iLen, _pszFormat, marker);
	va_end(marker);
	::OutputDebugString(_T("%s", pcData));
}

void __cdecl trace_d(const char* _pszFormat, ...)
{
#ifdef _DEBUG
	static const int iMaxPath = 512;
	char pcData[iMaxPath] = {0};
	int iLen = strlen(pcData);
	va_list marker;
	va_start(marker, _pszFormat);
	vsnprintf(pcData+iLen, iMaxPath-iLen, _pszFormat, marker);
	va_end(marker);
	::OutputDebugString(_T("%s", pcData));
#endif
}

int __cdecl CheckFileSize(const char* _pszFileName)
{
	
	FILE* fpTry = _fsopen(_pszFileName, "rb", _SH_DENYNO);
	if (NULL == fpTry)
	{
		return -1;
	}

	fseek(fpTry, 0, SEEK_END);
	unsigned int uiLength = ftell(fpTry);
	if (uiLength > INT_MAX)
	{
		fclose(fpTry);
		return -2;
	}
	fclose(fpTry);
	return 0;
}
std::string __cdecl IntToStr( int _iValue )
{
	char pcValue[32] = {0};
	std::sprintf(pcValue,"%d",_iValue);
	std::string strTemp(pcValue);
	return strTemp;
}
