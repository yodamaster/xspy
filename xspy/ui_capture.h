#pragma once
#include "atlmisc.h"
#include "common.h"
#include "utils.h"
#include <set>

// 模板仿wtl::CBitmapButtonImpl，因为我们要SubclassWindow，而Attach是不行的，因为有BEGIN_MSG_MAP
// 主要代码抄袭自wtl::CZoomScrollImpl和mfcspy
template <class T, class TBase = CStatic, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE ui_capture_t : public ATL::CWindowImpl< T, TBase, TWinTraits>
{
public:
    //// API 
    // 设置一个STATIC控件为捕捉窗口
    BOOL SubclassWindow(HWND hWnd);
    // 添加接收WM_SPY消息的窗口
    void AddRecvWnd(HWND hWnd);
    // 移除接收WM_SPY消息的窗口
    void RemoveRecvWnd(HWND hWnd);
public:
    ui_capture_t();
    ~ui_capture_t();

protected:
    BEGIN_MSG_MAP(ui_capture_t)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
	END_MSG_MAP()

    void Init();
    LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    VOID FrameWindow(HWND hWnd);
    void DrawFrame(POINT pt);

private:
	HICON m_hIcon;
    bool m_bTracking;
    HWND m_hRecvMsgWnd;
    HWND m_hLastWnd;
    CPen m_pen;
    HCURSOR cursor_sys;
    std::set<HWND> m_recvWnd;
};

#include "ui_capture_impl.h"

class ui_capture : public ui_capture_t<ui_capture>
{
public:
	DECLARE_WND_SUPERCLASS(_T("WTL_BitmapButton"), GetWndClassName())
};