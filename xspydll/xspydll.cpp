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
// xspydll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <string>
#include <boost/format.hpp>
#include "xspydll.h"
#include "common.h"
#include "mfc.h"
#include "atl.h"

HMODULE g_hModule = NULL;
#define NUM_WNDINFO 16*4*1024-16

// 每个数据都必须初始化
#pragma data_seg(".SharedDataName")
HHOOK g_hk = NULL;
HWND g_hDstWnd = NULL;
BOOL g_bOK = FALSE;
char g_dllname[MAX_PATH] = {0};
char g_hwndinfo[NUM_WNDINFO] = {0};
#pragma data_seg()
#pragma comment(linker,"/section:.SharedDataName,rws")

#define MKTRUE(x) ((x),true)

static BOOL DoSpyIt(HWND hWnd, std::string & sresult)
{
    BOOL bHasResult = TRUE; // 没信息也显示一些已经获取到的信息

    DWORD pid, tid;
    tid = GetWindowThreadProcessId(hWnd, &pid);
    sresult += boost::str(boost::format("target window pid: 0x%08x, thread tid: 0x%08x\r\n") % pid % tid);
    tid = GetCurrentThreadId();
    sresult += boost::str(boost::format("hook thread tid: 0x%08x\r\n") % tid);
    bHasResult = TRUE;

    SpyATL(hWnd, sresult);

    SpyMfc(hWnd, sresult);

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
