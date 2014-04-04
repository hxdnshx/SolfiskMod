#include "pch.hpp"
#include "DebugFunc.hpp"


#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"

#define MINIMAL_USE_PROCESSHEAPARRAY
#include "MinimalArray.hpp"
static wchar_t LogName[]=_T("Solfisk.log");

void WriteToLog(Minimal::MinimalStringT<wchar_t> str)
{
	HANDLE hf = CreateFileW(LogName,
											 GENERIC_WRITE|GENERIC_READ,
											 FILE_SHARE_WRITE,
											 NULL,
											 OPEN_ALWAYS,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL);
	if(FAILED(hf))
	{
		return;
	}
	DWORD dwPos = SetFilePointer(hf, 0, NULL, FILE_END); 
	DWORD dwSize=0;
	DWORD size=str.GetSize()*sizeof(wchar_t);
	DWORD r=WriteFile(hf,str.GetRaw(),size,&dwSize,NULL);
	if(FAILED(r))
	{
		int i=3;
	}
	CloseHandle(hf);
}

void WriteToLog(LPCVOID ptr,int size)
{
	HANDLE hf = CreateFileW(LogName,
											 GENERIC_WRITE|GENERIC_READ,
											 FILE_SHARE_WRITE,
											 NULL,
											 OPEN_ALWAYS,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL);
	if(!FAILED(hf))
	{
		return;
	}
	DWORD dwPos = SetFilePointer(hf, 0, NULL, FILE_END); 
	DWORD dwSize=0;
	WriteFile(hf,ptr,size,&dwSize,NULL);
	CloseHandle(hf);
}

void WriteToLog(LPCWSTR ptr)
{
	HANDLE hf = CreateFileW(LogName,
											 GENERIC_WRITE|GENERIC_READ,
											 FILE_SHARE_WRITE,
											 NULL,
											 OPEN_ALWAYS,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL);
	if(hf==INVALID_HANDLE_VALUE)
	{
		return;
	}
	DWORD dwPos = SetFilePointer(hf, 0, NULL, FILE_END); 
	DWORD dwSize=0;
	DWORD size=wcslen(ptr)*sizeof(wchar_t);
	DWORD r=WriteFile(hf,ptr,size,&dwSize,NULL);
	if(!r)
	{
		int i=3;
	}
	CloseHandle(hf);
}

void Initial_LogSys()
{
	HANDLE hf = CreateFileW(LogName,
											 GENERIC_WRITE|GENERIC_READ,
											 FILE_SHARE_WRITE,
											 NULL,
											 CREATE_ALWAYS,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL);
	if(FAILED(hf))
	{
		return;
	}
	DWORD dwSize=0;
	DWORD uni=0xFEFF;
	WriteFile(hf,&uni,sizeof(uni),&dwSize,NULL);
	CloseHandle(hf);
}

void Finalize_LogSys()
{

}