/************************************************************************/
/* 查看MFC版本类的虚函数的方法：
/d1reportAllClassLayout
*/
/************************************************************************/

#pragma once

#include <boost/format.hpp>

#define MAKE_VFN(x) \
    virtual const char* x() \
{ \
    return #x; \
}

#define PRINT_VFN(x) \
    result += boost::str(boost::format("[vtbl+0x%02X]%-24s= %s\r\n") % index % #x % GetCodes((LPVOID)*pStart)); \
    ++pStart; \
    ++index;


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

struct CRuntimeClass
{
    // Attributes
    LPCSTR m_lpszClassName;
    int m_nObjectSize;
    UINT m_wSchema; // schema number of the loaded class
    //CObject* (PASCAL* m_pfnCreateObject)(); // NULL => abstract class
    LPVOID m_pfnCreateObject;
    //#ifdef _AFXDLL
    CRuntimeClass* (PASCAL* m_pfnGetBaseClass)();
    //#else
    //    CRuntimeClass* m_pBaseClass;
    //#endif
    //
};

class CObject
{
public:
    // 该函数可以获取类名和类大小
    virtual CRuntimeClass* GetRuntimeClass() const{return 0;};
    MAKE_VFN(dtor);
    MAKE_VFN(Serialize);

    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        index = 0;
        PRINT_VFN(GetRuntimeClass);
        PRINT_VFN(dtor);
        PRINT_VFN(Serialize);
    }
};

class CObject_dbg
{
public:
    virtual CRuntimeClass* GetRuntimeClass() const {return 0;};
    MAKE_VFN(dtor);
    MAKE_VFN(Serialize);
    MAKE_VFN(AssertValid);
    MAKE_VFN(Dump);

    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        index = 0;
        PRINT_VFN(GetRuntimeClass);
        PRINT_VFN(dtor);
        PRINT_VFN(Serialize);
        PRINT_VFN(AssertValid);
        PRINT_VFN(Dump);
    }
};

template<class T>
class CCmdTarget : public T
{
public:
    MAKE_VFN(OnCmdMsg);
    MAKE_VFN(OnFinalRelease);
    MAKE_VFN(IsInvokeAllowed);
    MAKE_VFN(GetDispatchIID);
    MAKE_VFN(GetTypeInfoCount);
    MAKE_VFN(GetTypeLibCache);
    MAKE_VFN(GetTypeLib);
    virtual const AFX_MSGMAP* GetMessageMap(){return 0;}
    // 不调用的虚函数就没必要MAKE_VFN了

    //#define MAKE_MEMBER(x,fmt) sTemp.Format( "[+%02X]" #x "=" fmt "\r\n",(PBYTE)&(x) - (PBYTE)this,x);str += sTemp;
    //    sTemp.Format( "[+%02X]vtbl address=%s\r\n",0,(LPCSTR)GetMods(vtbl));str += sTemp;

    void get_member(PDWORD& pStart, DWORD& index, std::string& result)
    {
        PDWORD pBegin = (PDWORD)pStart;

#define PRINT_MEMBER(x) \
    result += boost::str(boost::format("[+0x%02X]" #x " = 0x%p\r\n") % index % ); \
    ++pStart; \
    ++index;

        PRINT_MEMBER(CCmdTarget::m_dwRef); // 64位分布不知道是什么样的
        PRINT_MEMBER(CCmdTarget::m_pOuterUnknown);
        PRINT_MEMBER(CCmdTarget::m_xInnerUnknown);
        PRINT_MEMBER(CCmdTarget::m_xDispatch.m_vtbl);
        //		MAKE_MEMBER(CCmdTarget::m_xDispatch.m_nOffset);
        PRINT_MEMBER(CCmdTarget::m_bResultExpected);
        PRINT_MEMBER(CCmdTarget::m_xConnPtContainer.m_vtbl);
        //		MAKE_MEMBER(CCmdTarget::m_xConnPtContainer.m_nOffset,"%p");
    }

    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        T::get_vfn_string(pStart, index, result);
        PRINT_VFN(OnCmdMsg);
        PRINT_VFN(OnFinalRelease);
        PRINT_VFN(IsInvokeAllowed);
        PRINT_VFN(GetDispatchIID);
        PRINT_VFN(GetTypeInfoCount);
        PRINT_VFN(GetTypeLibCache);
        PRINT_VFN(GetTypeLib);
        PRINT_VFN(GetMessageMap);
        PRINT_VFN(GetCommandMap);
        PRINT_VFN(GetDispatchMap);
        PRINT_VFN(GetConnectionMap);
        PRINT_VFN(GetInterfaceMap);
        PRINT_VFN(GetEventSinkMap);
        PRINT_VFN(OnCreateAggregates);
        PRINT_VFN(GetInterfaceHook);
        PRINT_VFN(GetExtraConnectionPoints);
        PRINT_VFN(GetConnectionHook);
    }

};


template<class dbg>
class CWnd42X : public CCmdTarget<dbg>
{
public:
    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        CCmdTarget<dbg>::get_vfn_string(pStart, index, result);
        PRINT_VFN(PreSubclassWindow);
        PRINT_VFN(Create);
        PRINT_VFN(DestroyWindow);
        PRINT_VFN(PreCreateWindow);
        PRINT_VFN(CalcWindowRect);
        PRINT_VFN(OnToolHitTest);
        PRINT_VFN(GetScrollBarCtrl);
        PRINT_VFN(WinHelpA);
        PRINT_VFN(ContinueModal);
        PRINT_VFN(EndModalLoop);
        PRINT_VFN(OnCommand);
        PRINT_VFN(OnNotify);
        PRINT_VFN(GetSuperWndProcAddr);
        PRINT_VFN(DoDataExchange);
        PRINT_VFN(BeginModalState);
        PRINT_VFN(EndModalState);
        PRINT_VFN(PreTranslateMessage);
        PRINT_VFN(OnAmbientProperty);
        PRINT_VFN(WindowProc);
        PRINT_VFN(OnWndMsg);
        PRINT_VFN(DefWindowProcA);
        PRINT_VFN(PostNcDestroy);
        PRINT_VFN(OnChildNotify);
        PRINT_VFN(CheckAutoCenter);
        PRINT_VFN(IsFrameWnd);
        PRINT_VFN(SetOccDialogInfo);
    }
};

template <class dbg>
class CDialog42X : public CWnd42X<dbg>
{
public:
    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        CWnd42X<dbg>::get_vfn_string(pStart, index, result);
        PRINT_VFN(DoModal);
        PRINT_VFN(OnInitDialog);
        PRINT_VFN(OnSetFont);
        PRINT_VFN(OnOK);
        PRINT_VFN(OnCancel);
        PRINT_VFN(PreInitDialog);
    }
};

template <class dbg>
class CWnd90X : public CCmdTarget<dbg>
{
public:
    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        CCmdTarget<dbg>::get_vfn_string(pStart, index, result);
        PRINT_VFN(PreSubclassWindow);
        PRINT_VFN(Create);
        PRINT_VFN(CreateEx);
        PRINT_VFN(CreateEx);
        PRINT_VFN(DestroyWindow);
        PRINT_VFN(PreCreateWindow);
        PRINT_VFN(CalcWindowRect);
        PRINT_VFN(GetMenu);
        PRINT_VFN(SetMenu);
        PRINT_VFN(OnToolHitTest);
        PRINT_VFN(GetScrollBarCtrl);
        PRINT_VFN(WinHelpA);
        PRINT_VFN(HtmlHelpA);
        PRINT_VFN(WinHelpInternal);
        PRINT_VFN(ContinueModal);
        PRINT_VFN(EndModalLoop);
        PRINT_VFN(EnsureStdObj);
        PRINT_VFN(get_accParent);
        PRINT_VFN(get_accChildCount);
        PRINT_VFN(get_accChild);
        PRINT_VFN(get_accName);
        PRINT_VFN(get_accValue);
        PRINT_VFN(get_accDescription);
        PRINT_VFN(get_accRole);
        PRINT_VFN(get_accState);
        PRINT_VFN(get_accHelp);
        PRINT_VFN(get_accHelpTopic);
        PRINT_VFN(get_accKeyboardShortcut);
        PRINT_VFN(get_accFocus);
        PRINT_VFN(get_accSelection);
        PRINT_VFN(get_accDefaultAction);
        PRINT_VFN(accSelect);
        PRINT_VFN(accLocation);
        PRINT_VFN(accNavigate);
        PRINT_VFN(accHitTest);
        PRINT_VFN(accDoDefaultAction);
        PRINT_VFN(put_accName);
        PRINT_VFN(put_accValue);
        PRINT_VFN(SetProxy);
        PRINT_VFN(CreateAccessibleProxy);
        PRINT_VFN(OnCommand);
        PRINT_VFN(OnNotify);
        PRINT_VFN(GetSuperWndProcAddr);
        PRINT_VFN(DoDataExchange);
        PRINT_VFN(BeginModalState);
        PRINT_VFN(EndModalState);
        PRINT_VFN(PreTranslateMessage);
        PRINT_VFN(OnAmbientProperty);
        PRINT_VFN(WindowProc);
        PRINT_VFN(OnWndMsg);
        PRINT_VFN(DefWindowProcA);
        PRINT_VFN(PostNcDestroy);
        PRINT_VFN(OnChildNotify);
        PRINT_VFN(CheckAutoCenter);
        PRINT_VFN(IsFrameWnd);
        PRINT_VFN(CreateControlContainer);
        PRINT_VFN(CreateControlSite);
        PRINT_VFN(SetOccDialogInfo);
        PRINT_VFN(GetOccDialogInfo);
    }
};

template<class dbg>
class CDialog90X : public CWnd90X<dbg>
{
public:
    void get_vfn_string(PDWORD& pStart, DWORD& index, std::string& result)
    {
        CWnd90X<dbg>::get_vfn_string(pStart, index, result);
        PRINT_VFN(Create);
        PRINT_VFN(Create);
        PRINT_VFN(CreateIndirect);
        PRINT_VFN(CreateIndirect);
        PRINT_VFN(DoModal);
        PRINT_VFN(OnInitDialog);
        PRINT_VFN(OnSetFont);
        PRINT_VFN(OnOK);
        PRINT_VFN(OnCancel);
        PRINT_VFN(PreInitDialog);
    }
};

// vc60到vs2008，CCmdTarget的虚函数表都是一样，vs2008的CWnd类虚函数表就多很多函数了
template <class dbg>
class CWndX : public CCmdTarget<dbg>
{
public:
};
template <class dbg>
class CDialogX : public CWndX<dbg>
{
public:
};
// 通用版本，不需要判断具体是哪个MFC版本，比如调用GetRuntimeClass的时候
typedef CWndX<CObject_dbg> CWndd;
typedef CWndX<CObject> CWnd;

typedef CDialogX<CObject_dbg> CDialogd;
typedef CDialogX<CObject> CDialog;

// 默认，对不支持的版本
typedef CWnd CWnd00;
typedef CWndd CWnd00d;
typedef CDialog CDialog00;
typedef CDialogd CDialog00d;

// mfc42
typedef CDialog42X<CObject>     CDialog42;
typedef CDialog42X<CObject_dbg> CDialog42d;
typedef CWnd42X<CObject>        CWnd42;
typedef CWnd42X<CObject_dbg>    CWnd42d;

// mfc90
typedef CDialog90X<CObject>     CDialog90;
typedef CDialog90X<CObject_dbg> CDialog90d;
typedef CWnd90X<CObject>        CWnd90;
typedef CWnd90X<CObject_dbg>    CWnd90d;