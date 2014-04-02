#pragma once

struct _LVPARAMFIELD {
	LPARAM winningRate : 16;
	LPARAM charId : 8;
	LPARAM pad    : 8;
};

#define LVPARAMFIELD(lParam) (*reinterpret_cast<_LVPARAMFIELD*>(&(lParam)))

LRESULT ColoredRecordView_OnCustomDraw(HWND hwndParent, LPNMLVCUSTOMDRAW lpnlvCustomDraw);
LRESULT ColoredRecordView_OnCustomDrawWithRateColorization(HWND hwndParent, LPNMLVCUSTOMDRAW lpnlvCustomDraw);

// 前景色
#define FORE_COLOR 0x000000

// 偶数奇数の背景色
#define EVEN_COLOR 0xFFFFFF
#define ODD_COLOR  0xD0DCD0

// 勝敗引き分けの背景色
#define WIN_COLOR  0xD0DCD0
#define LOSE_COLOR 0xD0D0EC
#define DRAW_COLOR 0xFFFFFF
