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