#include "pch.hpp"
#include "Solfisk.hpp"
#include "resource.h"
#include "SortListView.hpp"
#include "ScoreLine.hpp"
#include "ComboDlg.hpp"
//#include "ColoredRecordView.cpp"
#include "DlgCommon.hpp"


#define MINIMAL_USE_PROCESSHEAPSTRING
#include "MinimalPath.hpp"

#define UC_REFLESH 0xEE00


SLVCOLUMN s_listColumns2[] = {
	{ LVCFMT_LEFT,  100, _T("プロフ"),	SLVSORT_STRING },//profile
	{ LVCFMT_RIGHT,  55, _T("キャラ"),	SLVSORT_STRING },//character
	{ LVCFMT_RIGHT,  65, _T("ダメージ"),		SLVSORT_INTEGER },//damage
	{ LVCFMT_RIGHT,  55, _T("スタン"),		SLVSORT_INTEGER },//stun
	{ LVCFMT_RIGHT,  55, _T("レート"),	SLVSORT_INTEGER },//rate
	{ LVCFMT_RIGHT,  55, _T("ヒット"),	SLVSORT_INTEGER }//hit
};

static void ComboDialog_QueryCallBack(COMBOINFO_ITEM *item,void *user)
{
	HWND listWnd = reinterpret_cast<HWND>(user);

	TCHAR text[256];
	LVITEM  lvitem;
	lvitem.mask=LVIF_TEXT;
	lvitem.iItem = 0;
	lvitem.iSubItem = 0;
	lvitem.pszText=text;
	::lstrcpyn(text,MinimalA2T(item->pname),_countof(text));
	ListView_InsertItem(listWnd,&lvitem);

	lvitem.iSubItem = 1;
	lvitem.pszText = const_cast<LPTSTR>(TH135AddrGetCharName(item->pid)->abbr);
	ListView_SetItem(listWnd, &lvitem);

	lvitem.pszText=text;

	text[0]='\0';
	lvitem.iSubItem = 2;
	::wsprintf(text, _T("%d"), item->damage);
	ListView_SetItem(listWnd, &lvitem);

	text[0]='\0';
	lvitem.iSubItem = 3;
	::wsprintf(text, _T("%d"), item->stun);
	ListView_SetItem(listWnd, &lvitem);

	text[0]='\0';
	lvitem.iSubItem = 4;
	::wsprintf(text, _T("%d"), item->rate);
	ListView_SetItem(listWnd, &lvitem);

	text[0]='\0';
	lvitem.iSubItem = 5;
	::wsprintf(text, _T("%d"), item->hit);
	ListView_SetItem(listWnd, &lvitem);

}

static void SysMenu_OnClose(HWND hDlg, int x, int y)
{
	::DestroyWindow(hDlg);
}

static void SysMenu_OnReflesh(HWND hDlg, int x, int y)
{
	HWND listWnd = ::GetDlgItem(hDlg, IDC_LIST_COMBOINFO);
	ListView_DeleteAllItems(listWnd);
	COMBOINFO_FILTER_DESC *pFilterDesc = 
		reinterpret_cast<COMBOINFO_FILTER_DESC *>(::GetWindowLongPtr(hDlg, GWL_USERDATA));
	ComboInfo_QueryRecord(*pFilterDesc, ComboDialog_QueryCallBack, (void*)listWnd);
}

static LRESULT ComboDialog_OnDoubleClick(HWND hwndParent, HWND hwnd)
{
	return FALSE;
}

static void ComboDialog_OnSysCommand(HWND hDlg, UINT nID, int x, int y)
{
	if (nID == SC_CLOSE) {
		SysMenu_OnClose(hDlg, x, y);
	} else if (nID == UC_REFLESH) {
		SysMenu_OnReflesh(hDlg, x, y);
	}
}

static LRESULT ComboDialog_OnNotify(HWND hDlg, int idCtrl, LPNMHDR pNMHdr)
{
	switch(idCtrl) {
	case IDC_LIST_COMBOINFO:
		switch(pNMHdr->code) {
		case NM_CUSTOMDRAW:
			//return ColoredRecordView_OnCustomDrawWithRateColorization(hDlg, reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHdr));
		case NM_DBLCLK:
			return ComboDialog_OnDoubleClick(hDlg, pNMHdr->hwndFrom);
		case LVN_COLUMNCLICK:
			return SortListView_OnColumnClick(hDlg, reinterpret_cast<LPNMLISTVIEW>(pNMHdr));
		}
		break;
	}
	return FALSE;
}

static BOOL ComboDialog_InitSysMenu(HWND hDlg)
{
	HMENU hSysMenu = ::GetSystemMenu(hDlg, FALSE);
	int itemIndex = 0;
	::InsertMenu(hSysMenu, itemIndex++, MF_STRING | MF_BYPOSITION, UC_REFLESH, _T("最新の情報に更新"));
	::InsertMenu(hSysMenu, itemIndex++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
	return TRUE;
}

static BOOL ComboDialog_InitListView(HWND hDlg, COMBOINFO_FILTER_DESC &filterDesc)
{
	HWND hwndListView = ::GetDlgItem(hDlg, IDC_LIST_COMBOINFO);
	SortListView_Initialize(hwndListView, s_listColumns2, _countof(s_listColumns2));

	ComboInfo_QueryRecord(filterDesc, ComboDialog_QueryCallBack, reinterpret_cast<void*>(hwndListView));
	return TRUE;
}

static BOOL ComboDialog_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	COMBOINFO_FILTER_DESC *pReqFilterDesc = reinterpret_cast<COMBOINFO_FILTER_DESC *>(lParam);
	COMBOINFO_FILTER_DESC *pFilterDesc = new COMBOINFO_FILTER_DESC;
	if (pReqFilterDesc != NULL) {
		*pFilterDesc = *pReqFilterDesc;
	} else {
		pFilterDesc->mask = 0;
	}
	::SetWindowLongPtr(hDlg, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pFilterDesc));

	::SetWindowText(hDlg, _T("Combo History"));
	ComboDialog_InitSysMenu(hDlg);
	ComboDialog_InitListView(hDlg, *pFilterDesc);

	::ShowWindow(hDlg, SW_SHOW);
	return TRUE;
}

static void ComboDialog_OnDestroy(HWND hDlg)
{
	COMBOINFO_FILTER_DESC *pFilterDesc = 
		reinterpret_cast<COMBOINFO_FILTER_DESC *>(::GetWindowLongPtr(hDlg, GWL_USERDATA));
	delete pFilterDesc;
}

static BOOL ComboDialog_OnSize(HWND hDlg, UINT nType, WORD nWidth, WORD nHeight)
{
	RECT clientRect;
	::GetClientRect(hDlg, &clientRect);
	::SetWindowPos(::GetDlgItem(hDlg, IDC_LIST_COMBOINFO), NULL, 0, 0,
		clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_NOZORDER | SWP_NOMOVE);
	return TRUE;
}


static BOOL ComboDialog_OnGetMinMaxInfo(HWND hDlg, LPMINMAXINFO minMaxInfo)
{
	minMaxInfo->ptMinTrackSize.x = 0;
	minMaxInfo->ptMinTrackSize.y = 0;
	return TRUE;
}

static BOOL CALLBACK ComboDialog_DlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg) {
	HANDLE_DLG_MSG(hDlg, WM_INITDIALOG, ComboDialog_OnInitDialog);
	HANDLE_DLG_MSG(hDlg, WM_SYSCOMMAND, ComboDialog_OnSysCommand);
	HANDLE_DLG_MSG(hDlg, WM_NOTIFY, ComboDialog_OnNotify);
	HANDLE_DLG_MSG(hDlg, WM_DESTROY, ComboDialog_OnDestroy);
	HANDLE_DLG_MSG(hDlg, WM_GETMINMAXINFO, ComboDialog_OnGetMinMaxInfo);
	HANDLE_DLG_MSG(hDlg, WM_SIZE, ComboDialog_OnSize);
	}
	return FALSE;
}


bool CALLBACK ComboDialog_ShowModeless(HWND hwndParent, COMBOINFO_FILTER_DESC *pFilterDesc)
{
	HWND hRankDlg = ::CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_COMBOINFO),
		hwndParent,
		ComboDialog_DlgProc,
		reinterpret_cast<LPARAM>(pFilterDesc));
	return hRankDlg != NULL;
}