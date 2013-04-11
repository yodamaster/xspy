// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlframe.h>
//#include "ui_mfc.h"
//#include "ui_atl.h"
#include "ui_capture.h"
#include <boost/shared_ptr.hpp>

//class CAboutDlg;

class CMainDlg 
    : public CDialogImpl<CMainDlg>
    , public CMessageFilter
    , public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnIdle();

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		//NOTIFY_HANDLER(IDC_TAB1, TCN_SELCHANGE, OnTcnSelchangeTab1)
        MESSAGE_HANDLER(WM_SPY, OnSpy)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_HANDLER(IDC_CHECK1, BN_CLICKED, OnBnClickedCheck1)
        COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    LRESULT OnSpy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//LRESULT OnTcnSelchangeTab1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
    void CloseDialog(int nVal);
	//int CalcTabHeight();

    void ShowChange(int index);

private:

	//CTabCtrl m_tab;
	//ui_mfc ui_mfc_;
	//ui_atl ui_atl_;
	ui_capture ui_capture_;
    LONG m_oldBottom;
    //boost::shared_ptr<CAboutDlg> ui_about_;
public:
    LRESULT OnBnClickedCheck1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
