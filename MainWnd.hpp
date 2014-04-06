#pragma once

#include "ProfileIO.hpp"

// マンボウ
#define WINDOW_CLASS _T("Solfisk")

#ifdef _SPECIAL

#define WINDOW_TEXT  _T("綺録帖 Rev.FDDC")

#else


#define WINDOW_TEXT  _T("綺録帖 Rev.5.38")

#endif

bool MainWindow_Initialize();
bool MainWindow_PreTranslateMessage(MSG &msg);
