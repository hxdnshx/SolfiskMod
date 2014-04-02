#include "pch.hpp"
#include "DlgCommon.hpp"
#include "Solfisk.hpp"
#include "ScoreLineQIBFilterDlg.hpp"
#include "ScoreLine.hpp"
#include "resource.h"

#define PROP_FILTER_RESULT _T("filterResult")

static BOOL ScoreLineQIBFilterDialog_OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	::SendDlgItemMessage(hDlg, IDC_EDIT_P1NAME, EM_SETLIMITTEXT, 16, 0);
	::SendDlgItemMessage(hDlg, IDC_EDIT_P2NAME, EM_SETLIMITTEXT, 16, 0);
	::SetProp(hDlg, PROP_FILTER_RESULT, (HANDLE)lParam);
	return TRUE;
}

static void ScoreLineQIBFilterDialog_OnOK(HWND hDlg)
{
	SYSTEMTIME sysTime;
	SCORELINE_FILTER_DESC filterDesc;
	SCORELINE_FILTER_DESC &result = *(SCORELINE_FILTER_DESC *)::GetProp(hDlg, PROP_FILTER_RESULT);
	::RemoveProp(hDlg, PROP_FILTER_RESULT);

	filterDesc.mask = 0;

	if (::SendDlgItemMessage(hDlg, IDC_CHECK_P1NAME, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		filterDesc.mask |= 
			(::SendDlgItemMessage(hDlg, IDC_CHECK_P1NAME_LIKE, BM_GETCHECK, 0, 0) & BST_CHECKED)
			? SCORELINE_FILTER__P1NAMELIKE
			: SCORELINE_FILTER__P1NAME;
		TCHAR fltP1Name[MAX_PATH];
		::GetDlgItemText(hDlg, IDC_EDIT_P1NAME, fltP1Name, MAX_PATH);
		::lstrcpynA(filterDesc.p1name, MinimalT2A(fltP1Name), _countof(filterDesc.p1name) - 1);
		filterDesc.p1name[_countof(filterDesc.p1name) - 1] = 0;
	}
	if (::SendDlgItemMessage(hDlg, IDC_CHECK_P2NAME, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		filterDesc.mask |= 
			(::SendDlgItemMessage(hDlg, IDC_CHECK_P2NAME_LIKE, BM_GETCHECK, 0, 0) & BST_CHECKED)
			? SCORELINE_FILTER__P2NAMELIKE
			: SCORELINE_FILTER__P2NAME;
		TCHAR fltP2Name[MAX_PATH];
		::GetDlgItemText(hDlg, IDC_EDIT_P2NAME, fltP2Name, MAX_PATH);
		::lstrcpynA(filterDesc.p2name, MinimalT2A(fltP2Name), _countof(filterDesc.p2name) - 1);
		filterDesc.p2name[_countof(filterDesc.p2name) - 1] = 0;
	}

	if (::SendDlgItemMessage(hDlg, IDC_CHECK_DATEBEG, BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		filterDesc.mask |= SCORELINE_FILTER__TIMESTAMP_BEGIN;
		::SendDlgItemMessage(hDlg, IDC_DTP_DATEBEG, DTM_GETSYSTEMTIME, 0, (LPARAM)&sysTime);
		sysTime.wHour = sysTime.wMinute = sysTime.wSecond = sysTime.wMilliseconds = 0;
		::SystemTimeToFileTime(&sysTime, reinterpret_cast<LPFILETIME>(&filterDesc.t_begin));
	}
	if (::SendDlgItemMessage(hDlg, IDC_CHECK_DATEEND, BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		filterDesc.mask |= SCORELINE_FILTER__TIMESTAMP_END;
		::SendDlgItemMessage(hDlg, IDC_DTP_DATEEND, DTM_GETSYSTEMTIME, 0, (LPARAM)&sysTime);
		sysTime.wHour = 23;
		sysTime.wMinute = 59;
		sysTime.wSecond = 59;
		sysTime.wMilliseconds = 999;
		::SystemTimeToFileTime(&sysTime, reinterpret_cast<LPFILETIME>(&filterDesc.t_end));

		if ((filterDesc.mask & SCORELINE_FILTER__TIMESTAMP_BEGIN) && filterDesc.t_begin > filterDesc.t_end) {
			::MessageBox(hDlg, _T("始点は終点より前にしてください"), NULL, MB_OK | MB_ICONEXCLAMATION);
			return;
		}
	}

	result = filterDesc;
	::EndDialog(hDlg, IDOK);
}

static void ScoreLineQIBFilterDialog_OnCancel(HWND hDlg)
{
	::RemoveProp(hDlg, PROP_FILTER_RESULT);
	::EndDialog(hDlg, IDCANCEL);
}

static void ScoreLineQIBFilterDialog_OnCheck(HWND hDlg, int checkId, int targetId)
{
	LRESULT state = 
		::SendDlgItemMessage(hDlg, checkId, BM_GETCHECK, 0, 0);

	::EnableWindow(GetDlgItem(hDlg, targetId), state == BST_CHECKED);
}

static void ScoreLineQIBFilterDialog_OnCheckCouple(HWND hDlg, int checkId, int targetId1, int targetId2)
{
	LRESULT state = 
		::SendDlgItemMessage(hDlg, checkId, BM_GETCHECK, 0, 0);

	::EnableWindow(GetDlgItem(hDlg, targetId1), state == BST_CHECKED);
	::EnableWindow(GetDlgItem(hDlg, targetId2), state == BST_CHECKED);
}

static void ScoreLineQIBFilterDialog_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id) {
	case IDOK: ScoreLineQIBFilterDialog_OnOK(hDlg); break;
	case IDCANCEL: ScoreLineQIBFilterDialog_OnCancel(hDlg); break;
	case IDC_CHECK_P1NAME: ScoreLineQIBFilterDialog_OnCheckCouple(hDlg, id, IDC_EDIT_P1NAME, IDC_CHECK_P1NAME_LIKE); break;
	case IDC_CHECK_P2NAME: ScoreLineQIBFilterDialog_OnCheckCouple(hDlg, id, IDC_EDIT_P2NAME, IDC_CHECK_P2NAME_LIKE); break;
	case IDC_CHECK_DATEBEG: ScoreLineQIBFilterDialog_OnCheck(hDlg, id, IDC_DTP_DATEBEG); break;
	case IDC_CHECK_DATEEND: ScoreLineQIBFilterDialog_OnCheck(hDlg, id, IDC_DTP_DATEEND); break;
	}
}

BOOL CALLBACK ScoreLineQIBFilterDialog_DlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg) {
	HANDLE_DLG_MSG(hDlg, WM_INITDIALOG, ScoreLineQIBFilterDialog_OnInitDialog);
	HANDLE_DLG_MSG(hDlg, WM_COMMAND, ScoreLineQIBFilterDialog_OnCommand);
	}
	return FALSE;
}

int ScoreLineQIBFilterDialog_ShowModal(HWND hwndParent, LPVOID lpUser)
{
	return ::DialogBoxParam(
		g_hInstance, MAKEINTRESOURCE(IDD_QIBFILTER), hwndParent,
		ScoreLineQIBFilterDialog_DlgProc, (LPARAM)lpUser);
}
