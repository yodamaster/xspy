#pragma once

#define  WM_SPY  (WM_USER + 1)
#define  WM_SPY_START  (WM_USER + 1)

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
