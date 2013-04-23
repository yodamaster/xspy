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

#include "stdafx.h"
#include "atl.h"
#include <atlstdthunk.h>
#include <boost/format.hpp>
#include "common.h"

static BOOL TryCopyMemory(LPVOID dest, LPCVOID src, size_t size)
{
    if (!IsBadReadPtr(src, size))
    {
        memcpy(dest, src, size);
        return TRUE;
    }
    return FALSE;
}

//#include <types.h>
static BOOL TryReadThunkData(LPVOID dlgProc, std::string & sresult, bool bDlg = true)
{
    BOOL bHasResult = FALSE; 
    if (dlgProc)
    {
        sresult += bDlg ? "got DWLP_DLGPROC" : "got GWLP_WNDPROC";
        sresult += boost::str(boost::format(" = 0x%p\r\n")% dlgProc);

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
                bHasResult = TRUE;

                sresult += bDlg ? "Dialog" : "Windows";
                sresult += boost::str(boost::format
                    (" thunk address = 0x%p\r\n"
                    "class intstance = 0x%p\r\n")
                    % dlgProc 
                    % td.thunk_this);
                sresult += bDlg ? "DialogProc" : "WindowProc";
                sresult += boost::str(boost::format("= %s\r\n") % GetCodes(td.thunk_proc));

                LPVOID lpVtable, lpMsgap;

                if (TryCopyMemory(&lpVtable, td.thunk_this, sizeof(lpVtable)))
                {
                    sresult += boost::str(boost::format("[00]vftable address = 0x%p\r\n") % lpVtable);
                    if (TryCopyMemory(&lpMsgap, lpVtable, sizeof(lpMsgap)))
                    {
                        sresult += boost::str(boost::format("[vtbl+00]ProcessWindowMessage = %s\r\n") % GetCodes(td.thunk_proc));
                    }
                }
            }
        }

    }
    return bHasResult;
}


void SpyATL( HWND hWnd, std::string& sresult)
{
    sresult += "----------获取ATL/WTL相关信息-------------\r\n";
    LONG_PTR winProc = ::GetWindowLongPtr(hWnd, DWLP_DLGPROC);
    if(TryReadThunkData((LPVOID)winProc, sresult))
    {
        //bHasResult = TRUE;
    }

    // 注意该调用总能获取到地址，要读取内存来判断
    winProc = ::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
    if(TryReadThunkData((LPVOID)winProc, sresult, false))
    {
        //bHasResult = TRUE;
    }
}

