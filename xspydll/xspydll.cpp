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

static bool g_Isdbg  = false;
static bool g_IsStatic = false;

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

//see afxtrace.cpp
// from http://stackoverflow.com/questions/2664703/macros-in-macros-c
#include <boost/preprocessor/control/if.hpp>
#define PP_CHECK_N(x, n, ...) n
#define PP_CHECK(...) PP_CHECK_N(__VA_ARGS__, 0,)
#ifdef _WIN64
#define WIN64_XXXX ~, 1,
#endif
#define POINTER_FORMAT_STR \
BOOST_PP_IF(PP_CHECK(WIN64_XXXX), "%p", "0x%08X")

// this doesn't work
//#define POINTER_FORMAT_STR #ifdef _WIN64 \
//    "%p" \
//    #else \
//    "0x%08X" \
//    #endif
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

//kmp搜索算法
void kmp_init(const unsigned char *patn, int len, int *next)
{
    int i, j;

    /*	assert(patn != NULL && len > 0 && next != NULL);
    */
    next[0] = 0;
    for (i = 1, j = 0; i < len; i++) {
        while (j > 0 && patn[j] != patn[i])
            j = next[j - 1];
        if (patn[j] == patn[i])
            j ++;
        next[i] = j;
    }
}

int kmp_find(const unsigned char *text, int text_len, const unsigned char *patn,
             int patn_len, int *next)
{
    int i, j;

    /*	assert(text != NULL && text_len > 0 && patn != NULL && patn_len > 0
    && next != NULL);
    */
    for (i = 0, j = 0; i < text_len; i ++ ) {
        while (j > 0 && text[i] != patn[j])
            j = next[j - 1];
        if (text[i] == patn[j])
            j ++;
        if (j == patn_len)
            return (i + 1 - patn_len);
    }

    return -1;//没有找到
}

#ifndef _WIN64
// dword ptr [ebp+0Ch],360h
const unsigned char magic_code[] = {
    0x81, 0x7D, 0x0C, 0x60, 0x03, 0x00, 0x00
};

// mfc42.dll特例，或许不止EDI ? EAX, ECX, EBX...
// MOV EDI,DWORD PTR SS:[EBP+0xC]
// CMP EDI,0x360
const unsigned char magic_code2[] = {
    0x8B, 0x7D, 0x0C, 0x81, 0xFF, 0x60, 0x03, 0x00, 0x00
};
#else

#endif


//// 加在这没用，要在sources里添加
#ifndef _WIN64
#pragma comment(lib, "LDE64.lib")
#else
#pragma comment(lib, "LDE64x64.lib")
#endif
// size_t在64位上就变成64位了!

extern "C" size_t __stdcall LDE(IN PVOID  Address, IN ULONG  x64);

#include <boost/shared_array.hpp>
LPVOID find_FromHandlePermanent(LPVOID start_addr, size_t start_len)
{
    int ret = -1;
    size_t mg_len;
    {
        mg_len = sizeof(magic_code);
        boost::shared_array<int> next(new int[mg_len]);
        kmp_init(magic_code, mg_len, next.get());
        ret = kmp_find((const unsigned char*)start_addr, (int)start_len, magic_code, mg_len, next.get());
    }
    
    // 再尝试
    if (ret == -1)
    {
        mg_len = sizeof(magic_code2);
        boost::shared_array<int> next(new int[mg_len]);
        kmp_init(magic_code2, mg_len, next.get());
        ret = kmp_find((const unsigned char*)start_addr, (int)start_len, magic_code2, mg_len, next.get());
    }

    if (ret == -1)
    {
        return 0;
    }
    
    unsigned char* pStart = ((unsigned char*)start_addr + ret + mg_len);

    for (int i = 0; i< 20; ++i)
    {
        if (*pStart == 0xE8) // call
        {
            INT_PTR reloc= *(INT_PTR*)(pStart + 1);
            return (LPVOID)((INT_PTR)pStart + 5 + reloc);
        }
#ifndef _WIN64
        size_t Length = LDE(pStart, 0);
#else
        size_t Length = LDE(pStart, 64);
#endif
        pStart += Length;
    }

    return 0;
}

struct CODE_SECTION_T
{
    LPVOID VirtualAddr;
    DWORD VirtualSize;
};
static std::vector<CODE_SECTION_T> GetCodeSection(HMODULE hMod)
{
    std::vector<CODE_SECTION_T> ret;

    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER *)hMod;
    IMAGE_NT_HEADERS *pNtHeader = (IMAGE_NT_HEADERS*)((BYTE *)hMod + pDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pish = IMAGE_FIRST_SECTION(pNtHeader);
    //IMAGE_SECTION_HEADER emptySection = {0};
    WORD nSections = pNtHeader->FileHeader.NumberOfSections;
    for (WORD i =  0; pish && (i < nSections); ++i)
    {
    //while(pish && 0 != memcmp(pish, &emptySection, sizeof(IMAGE_SECTION_HEADER))) // mfcspy里的函数是错误的，mfc42.dll就不是这样
    //{
        if ((pish->Characteristics & (IMAGE_SCN_CNT_CODE|IMAGE_SCN_MEM_EXECUTE) ) ==
            (IMAGE_SCN_CNT_CODE|IMAGE_SCN_MEM_EXECUTE))
        {
            CODE_SECTION_T c;
            c.VirtualAddr = (char*)hMod + pish->VirtualAddress;
            c.VirtualSize = pish->Misc.VirtualSize;
            ret.push_back(c);
        }

        ++pish;
    }
    return ret;
}
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
#ifndef WM_QUERYAFXWNDPROC
#define WM_QUERYAFXWNDPROC  0x0360
#endif


struct AFX_MSGMAP_ENTRY
{
    UINT nMessage;   // windows message
    UINT nCode;      // control code or WM_NOTIFY code
    UINT nID;        // control ID (or 0 for windows messages)
    UINT nLastID;    // used for entries specifying a range of control id's
    UINT_PTR nSig;       // signature type (action) or pointer to message #
    //AFX_PMSG pfn;    // routine to call (or special value)
    LPVOID pfn;
};

struct AFX_MSGMAP
{
    LPVOID pfnGetBaseMap;
    //const AFX_MSGMAP* (PASCAL* pfnGetBaseMap)();
    const AFX_MSGMAP_ENTRY* lpEntries;
};
typedef const AFX_MSGMAP* (*GETMESSAGEMAP)();
static void SpyMfc(HWND hWnd, std::string& result)
{
    BOOL bIsMfc = ::SendMessage(hWnd,WM_QUERYAFXWNDPROC,0,0);
    if (!bIsMfc)
    {
        result += "不是MFC窗口\r\n";
        return ;
    }

    // exe
    HMODULE hMod = NULL, hModSearch = NULL;
    if (0 == g_dllname[0])
    {
        hMod = ::GetModuleHandle(NULL);
    }
    else
        hMod = ::GetModuleHandleA(g_dllname);

    //const char* pBegin = (const char*)::GetModuleHandle(NULL);
    //DWORD dwPE = *((DWORD*)(pBegin + 0x3C)); // 指向PE签名 
    // dwPE + 4就可以判以后是否是64位，这里不用

    IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER *)hMod;
    IMAGE_NT_HEADERS *pNtHeader = (IMAGE_NT_HEADERS*)((BYTE *)hMod + pDosHeader->e_lfanew);
    IMAGE_OPTIONAL_HEADER *pOptHeader = &pNtHeader->OptionalHeader;
    IMAGE_IMPORT_DESCRIPTOR *pImportDesc = (IMAGE_IMPORT_DESCRIPTOR *) ((BYTE *)hMod + 
        pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    g_IsStatic = true;
    while(pImportDesc->FirstThunk)  
    {  
        const char *pszDllName = (const char *)((BYTE *)hMod + pImportDesc->Name);
        if (0 == strnicmp(pszDllName, "mfc", 3))
        {
            const char* pNum = pszDllName + 3;
            char* pEnd = 0;
            unsigned long v = strtoul(pNum, &pEnd, 10);
            if (v >= 40 && v < 200) // 现在最大是100吧
            {
                g_Isdbg = false;
                if (*pEnd == 'd' || *pEnd == 'D')
                {
                    g_Isdbg = true;
                    pEnd += 1;
                }
                if (0 == strnicmp(pEnd, ".dll", 4)) // 还有mfc90chs.dll样式的
                {
                    result += pszDllName;
                    g_IsStatic = false;
                    hModSearch = GetModuleHandleA(pszDllName);
                    break;
                }
            }
        }

        // printf("/n模块名称:%s/n", pszDllName);
        //IMAGE_THUNK_DATA *pThunk = (IMAGE_THUNK_DATA *)((BYTE *)hMod + pImportDesc->OriginalFirstThunk);  
        //int n = 0;  
        ////MessageBox(NULL, "Test", "MESS", MB_OK);  
        //char *pszFunName = NULL;  
        //while(pThunk->u1.Function)  
        //{  
        //    pszFunName = (char *)((BYTE *)hMod + (DWORD)pThunk->u1.AddressOfData + 2);  
        //    PDWORD lpAddr = (DWORD *)((BYTE *)hMod + pImportDesc->FirstThunk) + n;  
        //    try  
        //    {  
        //        printf("function name : %-25s", (char *)pszFunName);  
        //    }  
        //    catch(...)  
        //    {  
        //        printf("function name :unknown!");  
        //    }  
        //    printf("addr :%0X/n", lpAddr);  
        //    n++;  
        //    pThunk++;  
        //}  
        pImportDesc++;  
    }

    if (g_IsStatic)
    {
        hModSearch = hMod;
    }

    std::vector<CODE_SECTION_T> search_area = GetCodeSection(hModSearch);

    std::vector<CODE_SECTION_T>::const_iterator vi = search_area.begin();
    for (; vi != search_area.end(); ++vi)
    {
        //result += boost::str(boost::format("VirtualAddr ="
        //            POINTER_FORMAT_STR " VirtualSize=" POINTER_FORMAT_STR)%(*vi).VirtualAddr % (*vi).VirtualSize
        //           );
        LPVOID p = find_FromHandlePermanent((*vi).VirtualAddr, (*vi).VirtualSize);
        if (p)
        {
            result += boost::str(boost::format("FromHandlePermanent = " POINTER_FORMAT_STR "\r\n") % p);

            typedef PVOID  (__stdcall *FROMHANDLEPERMANENT)(HWND hWnd);
            FROMHANDLEPERMANENT FromHandlePermanent = (FROMHANDLEPERMANENT)p;
            p = FromHandlePermanent(hWnd);
            result += boost::str(boost::format("CWnd = " POINTER_FORMAT_STR "\r\n") % p); 
        }
        else
        {
            result += "failed to find CWnd::FromHandlePermanent!\r\n";
        }
    }

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
            std::string str;
            g_bOK = DoSpyIt(g_hDstWnd, str);
            if (g_bOK)
            {
                strncpy_s(g_hwndinfo, NUM_WNDINFO, str.c_str(), sizeof(g_hwndinfo)-1);
                g_hwndinfo[sizeof(g_hwndinfo)-1] = 0;
            }
            return g_bOK;
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
