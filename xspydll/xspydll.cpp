// xspydll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <string>
#include <boost/format.hpp>
#include <atlstdthunk.h>
#include "xspydll.h"
#include "mfc.h"
#include "common.h"

HMODULE g_hModule = NULL;
#define NUM_WNDINFO 16*4*1024-16

// 每个数据都必须初始化
#pragma data_seg(".SharedDataName")
HHOOK g_hk = NULL;
HWND g_hDstWnd = NULL;
BOOL g_bOK = FALSE;
UINT32 g_Type = 0; // mfc=0, atl=1
char g_dllname[MAX_PATH] = {0};
char g_hwndinfo[NUM_WNDINFO] = {0};
#pragma data_seg()
#pragma comment(linker,"/section:.SharedDataName,rws")

#define MKTRUE(x) ((x),true)


static BOOL TryCopyMemory(LPVOID dest, LPCVOID src, size_t size)
{
    if (!IsBadReadPtr(src, size))
    {
        memcpy(dest, src, size);
        return TRUE;
    }
    return FALSE;
}


static BOOL TryReadThunkData(LONG_PTR dlgProc, std::string & sresult, bool bDlg = true)
{
    BOOL bHasResult = FALSE; 
    if (dlgProc)
    {
        sresult += bDlg ? "DWLP_DLGPROC" : "GWLP_WNDPROC";
        sresult += boost::str(boost::format(" address: "
            POINTER_FORMAT_STR//"0x%08x"
            "\r\n") % dlgProc);

        ATL::_stdcallthunk thunk;
        if (TryCopyMemory(&thunk, (LPCVOID)dlgProc, sizeof(ATL::_stdcallthunk)))
        {
            struct thunk_data
            {
                LPVOID thunk_this;
                LPVOID thunk_proc;
                thunk_data() : thunk_this(0), thunk_proc(0){}
            };
            thunk_data td;

            // d:\Program Files\Microsoft Visual Studio 9.0\VC\atlmfc\include\atlstdthunk.h中的_stdcallthunk
#ifdef _WIN64
            if (0xb948 == thunk.RcxMov && 0xb848 == thunk.RaxMov && 0xe0ff == thunk.RaxJmp)
            {
                td.thunk_this = (LPVOID)thunk.RcxImm;
                td.thunk_proc = (LPVOID)thunk.RaxImm;
            }
#else
            if (thunk.m_mov == 0x042444C7 && thunk.m_jmp == 0xe9)
            {
                // 计算绝对地址，代码参考atlstdthunk.h
                td.thunk_proc = LPVOID(((INT_PTR)dlgProc+sizeof(ATL::_stdcallthunk)) + (INT_PTR)thunk.m_relproc);
                td.thunk_this = (LPVOID)thunk.m_this;
            }
#endif

            if (td.thunk_this && td.thunk_proc)
            {
                g_Type = 1;
                bHasResult = TRUE;
                
                sresult += bDlg ? "Dialog" : "Windows";
                sresult += boost::str(boost::format
                    (" thunk address = "
                    POINTER_FORMAT_STR//0x%08x
                    "\r\n"
                    "\tclass intstance = 0x%08x\r\n")
                    % dlgProc 
                    % td.thunk_this);
                sresult += bDlg ? "\tDialogProc" : "\tWindowProc";
                sresult += boost::str(boost::format("= "
                    POINTER_FORMAT_STR//0x%08x
                    "\r\n") % td.thunk_proc);

                LPVOID lpVtable, lpMsgap;

                if (TryCopyMemory(&lpVtable, td.thunk_this, sizeof(lpVtable)))
                {
                    sresult += boost::str(boost::format("\tvftable address = "
                        POINTER_FORMAT_STR//0x%08x
                        "\r\n") % lpVtable);
                    if (TryCopyMemory(&lpMsgap, lpVtable, sizeof(lpMsgap)))
                    {
                        sresult += boost::str(boost::format("\tmsg map address = "
                            POINTER_FORMAT_STR//0x%08x
                            "\r\n") % lpMsgap);
                    }
                }

                //sresult += boost::str(boost::format
                //    ("Dialog thunk address: 0x%08x\r\n"
                //    "\tclass intstance: 0x%08x\r\n"
                //    "\tDialogProc: 0x%08x\r\n"
                //    "\tvftable address: 0x%08x\r\n"
                //    "\tmsg map address: 0x%08x\r\n")
                //    % dlgProc 
                //    % td.thunk_this
                //    % td.thunk_proc
                //    % lpVtable
                //    % lpMsgap);
            }
        }

    }
    return bHasResult;
}



static BOOL DoSpyIt(HWND hWnd, std::string & sresult)
{
    BOOL bHasResult = FALSE;

    DWORD pid, tid;
    tid = GetWindowThreadProcessId(hWnd, &pid);
    sresult += boost::str(boost::format("target window pid: 0x%08x, thread tid: 0x%08x\r\n") % pid % tid);
    tid = GetCurrentThreadId();
    sresult += boost::str(boost::format("hook thread tid: 0x%08x\r\n") % tid);
    bHasResult = TRUE;

    LONG_PTR winProc = ::GetWindowLongPtr(hWnd, DWLP_DLGPROC);
    if(TryReadThunkData(winProc, sresult))
    {
        bHasResult = TRUE;
    }

    // 注意该调用总能获取到地址，要读取内存来判断
    winProc = ::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    if(TryReadThunkData(winProc, sresult, false))
    {
        bHasResult = TRUE;
    }

    SpyMfc(hWnd, sresult);
    //std::map<CWnd*,DWORD> wnds;
    //GetWnds(hWnd,wnds);
    //if(wnds.size() == 0)
    //{
    //    BOOL bIsMfc = ::SendMessage(hWnd,WM_QUERYAFXWNDPROC,0,0);
    //    sresult.Format("map %sHWND %p To CWnd* failed!",bIsMfc?"mfc ":"",hWnd);
    //    if(bIsMfc)
    //    {
    //        CString temp;
    //        temp.Format("\r\nworking pid=%p,tid=%p fun=%p",
    //            (PVOID)GetCurrentProcessId(),(PVOID)GetCurrentThreadId(),
    //            GetFinalAddr(CWnd::FromHandlePermanent));
    //        sresult += temp;
    //    }
    //    return FALSE;
    //}
    //else
    //{
    //    std::map<CWnd*,DWORD>::iterator p;
    //    for(p = wnds.begin(); p!=wnds.end(); ++p)
    //    {
    //        SpyHelper * sh = (SpyHelper*) p->first;
    //        sh->Flag() = p->second;
    //        sh->SpyDlg(sresult,hWnd);
    //    }
    //}

    return bHasResult;
}


LRESULT CALLBACK CallWndProc(int code,WPARAM wParam,LPARAM lParam)
{
    CWPSTRUCT * cs = (CWPSTRUCT*)lParam;

    if(code>=0 && cs)
    {
        if(cs->hwnd == g_hDstWnd && cs->message==WM_NULL &&
            cs->wParam == 0x19820820 && cs->lParam == 0x19831014)
        {
            //boost::shared_ptr<std::string> str(new std::string);
            std::string str;
            g_bOK = DoSpyIt(g_hDstWnd, str);
            if (g_bOK)
            {
                strncpy_s(g_hwndinfo, NUM_WNDINFO, str.c_str(), sizeof(g_hwndinfo)-1);
                g_hwndinfo[sizeof(g_hwndinfo)-1] = 0;
            }
            return g_bOK;
            //return CallNextHookEx(g_hk,code,wParam,lParam);
        }
    }
    return CallNextHookEx(g_hk,code,wParam,lParam);
}

BOOL InstallHook(HWND hWnd)
{
    DWORD pid,tid;
    HMODULE mod = g_hModule;

    if(	g_hk == NULL && IsWindow(hWnd) && 
        MKTRUE(tid = GetWindowThreadProcessId(hWnd,&pid)) &&
        pid != GetCurrentProcessId() && mod &&
        (g_hk = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc,mod,tid)) != NULL )
    {
        g_hDstWnd = hWnd;
        return TRUE;
    }

    return FALSE;
}

VOID UninstallHook()
{
    g_hDstWnd = NULL;
    if(g_hk)
    {
        UnhookWindowsHookEx(g_hk);
        g_hk = NULL;
    }
}

struct LockObject : public CRITICAL_SECTION
{
    LockObject(){InitializeCriticalSection(this);}
    ~LockObject(){DeleteCriticalSection(this);}
    void Lock(){EnterCriticalSection(this);}
    void UnLock(){LeaveCriticalSection(this);}
}g_lock;

result_struct* xspydll_Spy(const arg_struct* arg)
{
    char *result = 0;
    HWND hWnd = arg->hWnd;
    if( !IsWindow(hWnd) ) return 0;

    g_dllname[0] = 0;
    if (arg->mfc_dll_name[0])
    {
        strncpy_s(g_dllname, MAX_PATH, arg->mfc_dll_name,sizeof(g_dllname) - 1);
    }
    DWORD tid,pid;
    g_lock.Lock();
    //EXFLAGS::_xmap.clear();

    tid = GetWindowThreadProcessId(hWnd,&pid);
    if(pid == GetCurrentProcessId())
    {
        std::string sresult;
        if (DoSpyIt(hWnd, sresult))
        {
            result = _strdup(sresult.c_str());
        }
        
    }
    else
    {
        if( InstallHook(hWnd))
        {
            // reset
            g_hwndinfo[0] = 0;
            g_bOK = FALSE;

            // 返回值好像没用
            SendMessage(hWnd, WM_NULL, 0x19820820, 0x19831014);
            if (g_bOK)
            {
                result = _strdup(g_hwndinfo);
            }
        }
        UninstallHook();
    }
    g_lock.UnLock();

    if (result)
    {
        result_struct* d = new result_struct;
        d->retMsg = result;
        d->type = g_Type;
        return d;
    }

    return 0;
}

void xspydll_SpyFree(result_struct* d)
{
    if (d)
    {
        free(d->retMsg);
        delete d;
    }
}
