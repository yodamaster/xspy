template <class T, class TBase, class TWinTraits>
ui_capture_t<T, TBase, TWinTraits>::ui_capture_t() : m_bTracking(false) , m_hRecvMsgWnd(NULL) , m_hLastWnd(NULL)
{

}

template <class T, class TBase, class TWinTraits>
ui_capture_t<T, TBase, TWinTraits>::~ui_capture_t()
{
    DestroyIcon(m_hIcon);

}

template <class T, class TBase, class TWinTraits>
BOOL ui_capture_t<T, TBase, TWinTraits>::SubclassWindow( HWND hWnd )
{
#if (_MSC_VER >= 1300)
    BOOL bRet = ATL::CWindowImpl< T, TBase, TWinTraits>::SubclassWindow(hWnd);
#else // !(_MSC_VER >= 1300)
    typedef ATL::CWindowImpl< T, TBase, TWinTraits>   _baseClass;
    BOOL bRet = _baseClass::SubclassWindow(hWnd);
#endif // !(_MSC_VER >= 1300)
    if(bRet)
        Init();
    return bRet;
}

template <class T, class TBase, class TWinTraits>
void ui_capture_t<T, TBase, TWinTraits>::Init()
{
    DWORD dwStyle = GetWindowLong(GWL_STYLE);
    SetWindowLong(GWL_STYLE, dwStyle | 0x50001103);

    DWORD dwExStyle = GetWindowLong( GWL_EXSTYLE);
    SetWindowLong(GWL_EXSTYLE, dwStyle | 0X20004);

    m_hIcon = (HICON)::LoadImage(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0); 
    ModifyStyle(0x0, SS_ICON | SS_CENTERIMAGE); 
    SetIcon(m_hIcon); 

    m_pen = (HPEN)::GetStockObject(WHITE_PEN);
}

template <class T, class TBase, class TWinTraits>
LRESULT ui_capture_t<T, TBase, TWinTraits>::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    std::set<HWND>::const_iterator si = m_recvWnd.begin();
    for (; si != m_recvWnd.end(); ++si)
    {
        ::PostMessage(*si, WM_SPY_START, 0, NULL);
    }
    if (!m_bTracking)
    {


        m_bTracking = true;
        SetCapture();
        SetIcon(NULL);
        ::SetCursor((HCURSOR)m_hIcon);
    }
    return 0;
}
template <class T, class TBase, class TWinTraits>
LRESULT ui_capture_t<T, TBase, TWinTraits>::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    ::ReleaseCapture();

    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    ::ClientToScreen(m_hWnd, &pt);
    HWND hWnd = ::GetWindowFromPointEx(pt);

    std::set<HWND>::const_iterator si = m_recvWnd.begin();
    for (; si != m_recvWnd.end(); ++si)
    {
        ::PostMessage(*si, WM_SPY, (WPARAM)hWnd, NULL);
    }
    return 0;
}

template <class T, class TBase, class TWinTraits>
LRESULT ui_capture_t<T, TBase, TWinTraits>::OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if(m_bTracking)
    {
        m_bTracking = false;
        ::SetCursor(cursor_sys);
        SetIcon(m_hIcon);
        if(m_hLastWnd) FrameWindow(m_hLastWnd);
        m_hLastWnd = NULL;
    }
    return 0;
}

template <class T, class TBase, class TWinTraits>
LRESULT ui_capture_t<T, TBase, TWinTraits>::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    if (m_bTracking == true && (wParam & MK_LBUTTON))
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ::ClientToScreen(m_hWnd, &pt);
        DrawFrame(pt);
    }
    return 0;
}

template <class T, class TBase, class TWinTraits>
void ui_capture_t<T, TBase, TWinTraits>::DrawFrame(POINT pt)
{
    HWND hWnd = GetWindowFromPointEx(pt);
    if(hWnd == m_hLastWnd) return;
    if(m_hLastWnd) FrameWindow(m_hLastWnd);
    FrameWindow(hWnd);
    m_hLastWnd = hWnd;
}

template <class T, class TBase, class TWinTraits>
VOID ui_capture_t<T, TBase, TWinTraits>::FrameWindow(HWND hWnd)
{
    RECT rc;
    ::GetWindowRect(hWnd,&rc);
    int w = rc.right - rc.left-1;
    int h = rc.bottom - rc.top-1;
    HDC hDC = ::GetWindowDC(hWnd);
    ::SetROP2(hDC,R2_XORPEN);
    HPEN hOldPen = (HPEN)::SelectObject(hDC,m_pen);
#define BB 4
    for(int i=0;i<4;++i)
    {
        ::MoveToEx(hDC,0,i,0); ::LineTo(hDC,w+1,i);
        ::MoveToEx(hDC,0,h-i,0); ::LineTo(hDC,w+1,h-i);
        ::MoveToEx(hDC,i,BB,0); ::LineTo(hDC,i,h-BB+1);
        ::MoveToEx(hDC,w-i,BB,0); ::LineTo(hDC,w-i,h-BB+1);
    }
    ::SelectObject(hDC,hOldPen);
    ::ReleaseDC(hWnd,hDC);
}

template <class T, class TBase, class TWinTraits>
void ui_capture_t<T, TBase, TWinTraits>::AddRecvWnd( HWND hWnd )
{
    m_recvWnd.insert(hWnd);
}

template <class T, class TBase , class TWinTraits >
void ui_capture_t<T, TBase, TWinTraits>::RemoveRecvWnd( HWND hWnd )
{
    m_recvWnd.erase(hWnd);
}