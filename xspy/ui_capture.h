/* 
 *
 * This file is part of xspy
 * By lynnux <lynnux@qq.com>
 * Copyright 2013 lynnux
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

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