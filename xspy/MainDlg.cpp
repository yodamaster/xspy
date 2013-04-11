// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"
#include "atlmisc.h"
#include "utils.h"
#include <string>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <atlstr.h>
#include "../xspydll/xspydll.h"
#ifdef _DEBUG
#ifdef _M_AMD64
#pragma comment(lib, "..\\Debug\\xspydll-x64.lib")
#else
#pragma comment(lib, "..\\Debug\\xspydll.lib")
#endif
#else
#ifdef _M_AMD64
#pragma comment(lib, "..\\Release\\xspydll-x64.lib")
#else
#pragma comment(lib, "..\\Release\\xspydll.lib")
#endif
#endif

//// 计算tab高度 from wtl:CTabView
//int CMainDlg::CalcTabHeight()
//{
//	int nCount = m_tab.GetItemCount();
//	int nIndex = m_tab.InsertItem(nCount, _T("NS"));
//
//	RECT rect = { 0, 0, 1000, 1000 };
//	m_tab.AdjustRect(FALSE, &rect);
//
//	RECT rcWnd = { 0, 0, 1000, rect.top };
//	::AdjustWindowRectEx(&rcWnd, m_tab.GetStyle(), FALSE, m_tab.GetExStyle());
//
//	int nHeight = rcWnd.bottom - rcWnd.top;
//
//	m_tab.DeleteItem(nIndex);
//
//	return nHeight;
//}

//class CAboutDlg : public CDialogImpl<CAboutDlg>
//{
//public:
//    enum { IDD = IDD_ABOUTBOX };
//    BEGIN_MSG_MAP(CAboutDlg)
//    END_MSG_MAP()
//};

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);
    // UIAddChildWindowContainer(m_hWnd); // for update

	// init
	ui_capture_.SubclassWindow(GetDlgItem(IDC_STATIC1));

    GetDlgItem(IDC_EDIT2).EnableWindow(FALSE);
    CButton b(GetDlgItem(IDC_CHECK1));
    b.SetCheck(BST_UNCHECKED);

    CRect rc;
    CRect rcShow;

    GetWindowRect(rc);
    GetDlgItem(IDC_STATIC_MORE).GetWindowRect(rcShow);
    m_oldBottom = rc.bottom - rcShow.bottom;
    rc.bottom = rcShow.bottom;
    MoveWindow(rc);
	//m_tab.Attach(GetDlgItem(IDC_TAB1));
	//m_tab.InsertItem(0, _T("MFC"));
	//m_tab.InsertItem(1, _T("ATL/WTL"));
    //m_tab.InsertItem(2, _T("关于"));

	//WTL::CRect rc;
	//m_tab.GetClientRect(rc);
	//rc.top += CalcTabHeight();
	//rc.bottom -= 5;
	//rc.left += 5;
	//rc.right -= 5;

	//ui_mfc_.Create(m_tab.m_hWnd);
	//ui_atl_.Create(m_tab.m_hWnd);
 //   ui_about_.reset(new CAboutDlg());
 //   ui_about_->Create(m_tab.m_hWnd);

	//ui_mfc_.MoveWindow(&rc);
	//ui_atl_.MoveWindow(&rc);
 //   ui_about_->MoveWindow(&rc);

    //ShowChange(0);

    //ui_capture_.AddRecvWnd(ui_mfc_);
    //ui_capture_.AddRecvWnd(ui_wtl_);
    ui_capture_.AddRecvWnd(m_hWnd); // 由主界面统一处理

    //EnablePrivilege();
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);
    return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ATL::CString text;
    GetDlgItem(IDC_EDIT1).GetWindowText(text);
    if (!text.IsEmpty())
    {
        if (text.GetLength() >= 2)
        {
            if (!(text.GetAt(0) == _T('0')
                && text.GetAt(1) == _T('x')))
            {
                text.Insert(0, _T("0x"));
            }
        }
        else
            text.Insert(0, _T("0x"));
        HWND hWnd;
#ifdef _M_AMD64 // 使用StrToInt64Ex，需要#define _WIN32_IE	0x0600
        StrToInt64Ex(text, STIF_SUPPORT_HEX, (LONGLONG *)&hWnd);
#else
        StrToIntEx(text, STIF_SUPPORT_HEX, (int*)&hWnd);
#endif
        BOOL b;
        OnSpy(0, (WPARAM)hWnd, 0, b);
    }
    return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//EndDialog(wID);
    CloseDialog(wID);
	return 0;
}

//LRESULT CMainDlg::OnTcnSelchangeTab1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
//{
//	ShowChange(m_tab.GetCurSel());
//	return 0;
//}

LRESULT CMainDlg::OnSpy( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{
    HWND hWnd = (HWND)wParam;
    if(hWnd==NULL || !::IsWindow(hWnd)){MessageBeep(0);return 0;}

    if (hWnd == GetDlgItem(IDC_STATIC1).m_hWnd)
        return 0;


    ATL::CString str;
    str.Format(_T("%08X"), hWnd);
    TCHAR cn[1024];
    if(GetClassName(hWnd, cn, sizeof(cn)))
    {
        str += "(";
        str += cn;
        str += ")";
    }
    if( ::GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD )
    {
        ATL::CString strTemp;
        HMENU menu = ::GetMenu(hWnd);
        strTemp.Format(_T(",id=%04x"), menu);
        str += strTemp;
    }
    
    GetDlgItem(IDC_EDIT1).SetWindowText(str);
    GetDlgItem(IDC_EDIT_MSG).SetWindowText(_T(""));

    // spy
    arg_struct arg;
    arg.hWnd = hWnd;
    arg.mfc_dll_name[0] = 0;
    CButton b(GetDlgItem(IDC_CHECK1));
    if(BST_CHECKED == b.GetCheck())
    {
        ATL::CString str;
        GetDlgItem(IDC_EDIT2).GetWindowText(str);
        if (!str.IsEmpty())
        {
#ifdef _UNICODE
            std::string str1 = ws2s((LPCTSTR)str);
            strncpy(arg.mfc_dll_name, str1.c_str(), sizeof(arg.mfc_dll_name) - 1);
#else
            strncpy(arg.mfc_dll_name, str, sizeof(arg.mfc_dll_name) - 1);
#endif
        }
    }
    boost::shared_ptr<result_struct> result;
    result.reset(xspydll_Spy(&arg), xspydll_SpyFree);
    if (result)
    {
        std::tstring strResult;

#ifdef _UNICODE
        strResult = s2ws(result->retMsg);
#else
        strResult = result->retMsg;
#endif
        GetDlgItem(IDC_EDIT_MSG).SetWindowText(strResult.c_str());

        //if (result->type == 0)
        //{
        //    SendMessage(ui_mfc_, WM_SPY, (WPARAM)strResult.c_str(), 0);
        //    ShowChange(0);
        //}
        //else if(result->type == 1)
        //{
        //    SendMessage(ui_atl_, WM_SPY, (WPARAM)strResult.c_str(), 0);
        //    ShowChange(1);
        //}
    }

    return 0;
}

//void CMainDlg::ShowChange( int index )
//{
//    ui_mfc_.ShowWindow(SW_HIDE);
//    ui_atl_.ShowWindow(SW_HIDE);
//    ui_about_->ShowWindow(SW_HIDE);
//
//    if (0 == index)
//        ui_mfc_.ShowWindow(SW_SHOW);
//    else if(1 == index)
//        ui_atl_.ShowWindow(SW_SHOW);
//    else if(2 == index)
//        ui_about_->ShowWindow(SW_SHOW);
//    
//    m_tab.SetCurSel(index);
//}

BOOL CMainDlg::PreTranslateMessage( MSG* pMsg )
{
    if(pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_RETURN)
        {
            HWND hWnd= ::GetFocus();
            if (IDC_EDIT1 == ::GetDlgCtrlID(hWnd))
            {
                BOOL b;
                OnOK(0, 0, 0, b);
                return FALSE;
            }
        }
    }

    return CWindow::IsDialogMessage(pMsg);
}

void CMainDlg::CloseDialog( int nVal )
{
    DestroyWindow();
    ::PostQuitMessage(nVal);
}

BOOL CMainDlg::OnIdle()
{
    return FALSE;
}

LRESULT CMainDlg::OnBnClickedCheck1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CButton b;
    b.Attach(GetDlgItem(IDC_CHECK1));
    if(BST_CHECKED == b.GetCheck())
    {
        GetDlgItem(IDC_EDIT2).EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT2).EnableWindow(FALSE);
    }
    return 0;
}


LRESULT CMainDlg::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    
    CRect rc;
    GetWindowRect(rc);
    rc.bottom += m_oldBottom;
    MoveWindow(rc);
    GetDlgItem(IDC_BUTTON1).ShowWindow(SW_HIDE);

    return 0;
}

