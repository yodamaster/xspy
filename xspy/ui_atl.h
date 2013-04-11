#pragma once
#include "resource.h"
#include "common.h" // WM_SPY

class ui_atl : public CDialogImpl<ui_atl>
{
public:
	ui_atl(void);
	~ui_atl(void);

	enum { IDD = IDD_ATL };

	BEGIN_MSG_MAP(ui_atl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_SPY, OnSpy)
    END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSpy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

};
