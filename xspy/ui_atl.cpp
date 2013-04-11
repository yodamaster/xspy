#include "StdAfx.h"
#include "ui_atl.h"
#include <string>
#include "utils.h"
ui_atl::ui_atl(void)
{
}

ui_atl::~ui_atl(void)
{
}

LRESULT ui_atl::OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{

	return TRUE;
}

LRESULT ui_atl::OnSpy( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
{


    GetDlgItem(IDC_EDIT1).SetWindowText((LPCTSTR)wParam);


    return 0;
}