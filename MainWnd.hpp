#pragma once

#include "ProfileIO.hpp"

// マンボウ
#define WINDOW_CLASS _T("Solfisk")
#define WINDOW_TEXT  _T("綺録帖 Rev.5.35")

bool MainWindow_Initialize();
bool MainWindow_PreTranslateMessage(MSG &msg);
