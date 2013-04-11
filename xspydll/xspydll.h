#pragma once

#ifdef XSPYDLL_EXPORTS
#define XSPYAPI __declspec(dllexport)
#else
#define XSPYAPI __declspec(dllimport)
#endif

#pragma pack(push, 1)
struct arg_struct
{
    HWND hWnd;
    char mfc_dll_name[MAX_PATH];
};
struct result_struct
{
    unsigned int type; // mfc = 0 , atl = 1
    char* retMsg;
};
#pragma pack(pop)

// Èç¹û
XSPYAPI result_struct* xspydll_Spy(const arg_struct* arg);
XSPYAPI void xspydll_SpyFree(result_struct*);