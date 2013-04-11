#pragma once

#define  WM_SPY  (WM_USER + 1)

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
