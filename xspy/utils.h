#pragma once
#include <windows.h>
#include <string>

HWND WINAPI GetWindowFromPointEx(POINT pt);

void EnablePrivilege(void);
std::string ws2s(const std::wstring& inputws);
std::wstring s2ws(const std::string& s);