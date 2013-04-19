#include "stdafx.h"
#include "common.h"
#include <boost/format.hpp>

std::string GetMods(LPCVOID addr)
{
    std::string s;
    MEMORY_BASIC_INFORMATION mb = {0};
    VirtualQuery(addr,&mb,sizeof(mb));
    PBYTE pb = (PBYTE)mb.AllocationBase;
    if(pb && pb[0] == 'M' && pb[1] == 'Z')
    {
        CHAR fn[MAX_PATH],*p;
        fn[0] = 0;
        GetModuleFileNameA((HMODULE)pb,fn,sizeof(fn));
        p = strrchr(fn,'\\');
        if(p) ++p ; else p = fn;
        s = boost::str(boost::format("0x%p(%s+ 0x%06X )") % addr % p % ((PBYTE)addr - (PBYTE)pb));
        //s.Format("%p(%s+%06X)",addr,p,(PBYTE)addr - (PBYTE)pb );
    }
    else
    {
        s = boost::str(boost::format("0x%p") % addr);
        //s.Format("%p",addr);
    }
    return s;
}

static PVOID GetFinalAddr( PVOID addr )
{
    PBYTE pbaddr = (PBYTE)addr;
    int changed = 0;
    do
    {
        changed = 0;
        if(pbaddr[0] == 0xe9)
        {
            pbaddr += 5 + *(PDWORD)(pbaddr+1);
            changed = 1;
        }
        else if(pbaddr[0] == 0xff && pbaddr[1] == 0x25)
        {
            pbaddr = **(PBYTE**)(pbaddr+2);
            changed = 1;
        }
    } while(changed);
    return pbaddr;
}

std::string GetCodes(PVOID addr)
{
    PVOID pbaddr = GetFinalAddr(addr);
    std::string s;
    if(pbaddr != addr)
    {
        s = boost::str(boost::format("0x%p -> ") % addr);
        //s.Format("%p->",addr);
    }
    s += GetMods(pbaddr);
    return s;
}
