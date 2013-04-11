#pragma once
#include "resource.h"

class ui_mfc : public CDialogImpl<ui_mfc>
{
public:
	ui_mfc(void);
	~ui_mfc(void);

	enum { IDD = IDD_MFC };

	BEGIN_MSG_MAP(ui_mfc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()


	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};
