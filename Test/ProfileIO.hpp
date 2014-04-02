#pragma once

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalString.hpp"

class CProfileIO
{
private:
	Minimal::ProcessHeapString m_path;

public:
	CProfileIO();
	CProfileIO(LPCTSTR path);

	BOOL SetPath(LPCTSTR path);
	LPCTSTR GetPath();
	DWORD ReadString(LPCTSTR sec, LPCTSTR key, LPCTSTR def, LPTSTR buf, DWORD len);
	UINT ReadInteger(LPCTSTR sec, LPCTSTR key, INT def);
	DWORD WriteString(LPCTSTR sec, LPCTSTR key, LPCTSTR value);
	DWORD WriteInteger(LPCTSTR sec, LPCTSTR key, INT value);
};
