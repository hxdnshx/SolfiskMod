#include "pch.hpp"
#include "ProfileIO.hpp"

CProfileIO::CProfileIO()
{
}

CProfileIO::CProfileIO(LPCTSTR path)
{
	SetPath(path);
}

BOOL CProfileIO::SetPath(LPCTSTR path)
{
	m_path = path;
	return TRUE;
}

LPCTSTR CProfileIO::GetPath()
{
	return m_path;
}

DWORD CProfileIO::ReadString(LPCTSTR sec, LPCTSTR key, LPCTSTR def, LPTSTR buf, DWORD len)
{
	return ::GetPrivateProfileString(sec, key, def, buf, len, m_path);
}

UINT CProfileIO::ReadInteger(LPCTSTR sec, LPCTSTR key, INT def)
{
	return ::GetPrivateProfileInt(sec, key, def, m_path);
}

DWORD CProfileIO::WriteString(LPCTSTR sec, LPCTSTR key, LPCTSTR value)
{
	return ::WritePrivateProfileString(sec, key, value, m_path);
}

DWORD CProfileIO::WriteInteger(LPCTSTR sec, LPCTSTR key, INT value)
{
	TCHAR buff[16];
	::wnsprintf(buff, sizeof buff, _T("%d"), value);
	return ::WritePrivateProfileString(sec, key, buff, m_path);
}

