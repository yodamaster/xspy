#include "StdAfx.h"
#include "utils.h"

struct MyData
{
    int x;
    int y;
    HWND hFindit;
    DWORD SearchData;
};

static BOOL CALLBACK EnumChildWindowProc(HWND hwnd,LPARAM lParam)
{
    RECT rc;
    struct MyData* p = (struct MyData*)lParam;
    if(!IsWindowVisible(hwnd))return TRUE;
    GetWindowRect(hwnd,&rc);
    if( p->x >= rc.left  && p->y >= rc.top  &&
        p->x <= rc.right &&	p->y <= rc.bottom )
    {
        DWORD d =(DWORD)(rc.right - rc.left);
        if( d < p->SearchData)
        {
            p->hFindit = hwnd;
            p->SearchData = rc.right-rc.left;
        }
    }
    return TRUE;
}

static HWND SearchWindow (HWND hWnd,POINT pt)
{
    struct MyData md = {pt.x,pt.y,0,-1};
    EnumChildWindows(hWnd,EnumChildWindowProc,(LPARAM)&md);
    if(md.hFindit == NULL || md.hFindit == hWnd)return hWnd;
    return SearchWindow(md.hFindit,pt);
}

HWND WINAPI GetWindowFromPointEx(POINT pt)
{
    HWND hWnd;
    DWORD style;

    hWnd = WindowFromPoint(pt);
    if(hWnd)
    {
        POINT pt2,ptx;
        ptx.x = ptx.y = 0;

        ClientToScreen(hWnd,&ptx);
        pt2.x = pt.x - ptx.x;
        pt2.y = pt.y - ptx.y;
        HWND hs1 = ChildWindowFromPoint(hWnd,pt2);
        if(hs1)hWnd = hs1;
    }
    if(hWnd==NULL)return NULL;
    LPARAM lp = MAKELPARAM(pt.x,pt.y);
    ULONG_PTR result=0;
    style = GetWindowLong(hWnd,GWL_STYLE);
    if( (style & WS_CHILDWINDOW)!=0 &&
        SendMessageTimeout(hWnd,WM_NCHITTEST,0,lp,SMTO_ABORTIFHUNG,1000,&result) && 
        result == HTTRANSPARENT)
    {
        hWnd =  SearchWindow(GetParent(hWnd),pt);
    }
    return hWnd;
}

void EnablePrivilege(void)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp = {0};
    HANDLE hProcess = GetCurrentProcess();
    if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &hToken))
        return;
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
    {
        CloseHandle(hToken);
        return;
    }
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
        NULL, NULL);
    CloseHandle(hToken);
}
#include <iterator>
//#pragma warning(push)
//#pragma warning(disable: 4244) // possible loss of data

std::string ws2s(const std::wstring& s){
    std::string temp;
    std::copy(s.begin(), s.end(), std::back_inserter(temp));
    return temp; 
}
//#pragma warning(pop)

std::wstring s2ws(const std::string& s){ 
    std::wstring str2;
    std::copy(s.begin(), s.end(), std::back_inserter(str2));
    return str2;
}
