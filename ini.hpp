/////////////////////////////////////
//
//    ini.hpp
//    ini ÎÄ¼þ²Ù×÷¿â
//    
//    by huidong <mailhuid@163.com> 
//    2023.02.02
//

#pragma once

#include <Windows.h>
#include <string>

inline int GetIniFileInfoInt(LPCTSTR lpFileName, LPCTSTR lpAppName, LPCTSTR lpKeyName, int nDefault)
{
	return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, lpFileName);
}

inline double GetIniFileInfoFloat(LPCTSTR lpFileName, LPCTSTR lpAppName, LPCTSTR lpKeyName, double dDefault)
{
	WCHAR buf[128] = { 0 };
	GetPrivateProfileString(lpAppName, lpKeyName, L"err", buf, sizeof(buf) / sizeof(WCHAR), lpFileName);
	if ((std::wstring)buf == L"err")    return dDefault;
	else                                return _wtof(buf);
}

inline std::wstring GetIniFileInfoString(LPCTSTR lpFileName, LPCTSTR lpAppName, LPCTSTR lpKeyName, std::wstring strDefault, int nMaxSize = 1024)
{
	WCHAR* buf = new WCHAR[nMaxSize];
	memset(buf, 0, sizeof(WCHAR) * nMaxSize);
	GetPrivateProfileString(lpAppName, lpKeyName, strDefault.c_str(), buf, nMaxSize, lpFileName);
	std::wstring str = buf;
	delete[] buf;
	return str;
}

inline bool WriteIniFileInfoInt(LPCTSTR lpFileName, LPCTSTR lpAppName, LPCTSTR lpKeyName, int nValue)
{
	WCHAR buf[12] = { 0 };
	_itow_s(nValue, buf, 10);
	return WritePrivateProfileString(lpAppName, lpKeyName, buf, lpFileName);
}

inline bool WriteIniFileInfoFloat(LPCTSTR lpFileName, LPCTSTR lpAppName, LPCTSTR lpKeyName, double dValue)
{
	WCHAR buf[12] = { 0 };
	swprintf(buf, 12, L"%lf", dValue);
	return WritePrivateProfileString(lpAppName, lpKeyName, buf, lpFileName);
}

inline bool WriteIniFileInfoString(LPCTSTR lpFileName, LPCTSTR lpAppName, LPCTSTR lpKeyName, std::wstring strValue)
{
	return WritePrivateProfileString(lpAppName, lpKeyName, strValue.c_str(), lpFileName);
}