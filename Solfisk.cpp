#include "pch.hpp"
#include "Solfisk.hpp"
#include "QIBSettings.hpp"
#include "Mainwnd.hpp"
#include "Shortcut.hpp"

#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"
#include "MinimalArray.hpp"

// シングルトン代わり
HINSTANCE g_hInstance;
Minimal::ProcessHeapString g_appPath;
CProfileIO g_settings;

static bool Application_PreTranslateMessage(MSG &msg)
{
	return MainWindow_PreTranslateMessage(msg);
}

static LPARAM Application_Run()
{
	MSG msg;
	while(::GetMessage(&msg, NULL, 0, 0) > 0) {
		if (Application_PreTranslateMessage(msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	return msg.lParam;
}

#ifndef _DEBUG
int main()
{
	g_hInstance = (HINSTANCE)GetModuleHandle(NULL);

	TCHAR appPath[1025];
	::GetModuleFileName(GetModuleHandle(NULL), appPath, sizeof appPath);
	::PathRemoveFileSpec(appPath);
	g_appPath = appPath;

	Minimal::ProcessHeapPath settingPath = g_appPath;
	settingPath /= _T("solfisk.ini");
	g_settings.SetPath(settingPath);

	return !MainWindow_Initialize() ? -1
		: Application_Run();
}
#else
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	g_hInstance = (HINSTANCE)GetModuleHandle(NULL);

	TCHAR appPath[1025];
	::GetModuleFileName(GetModuleHandle(NULL), appPath, sizeof appPath);
	::PathRemoveFileSpec(appPath);
	g_appPath = appPath;

	Minimal::ProcessHeapPath settingPath = g_appPath;
	settingPath /= _T("tsk.ini");
	OutputDebugString(settingPath);
	g_settings.SetPath(settingPath);

	return 	
		!MainWindow_Initialize() ? -1
		: Application_Run();
}
#endif
