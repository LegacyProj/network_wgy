// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once


#include <valarray>
#include <iostream>
#include <iomanip>

#include <tchar.h>
#include <windows.h>
#include <time.h>
#define SEND 0xf0000000
#define ACK 0x0f000000
#define CONF 0x00f00000

extern DWORD dwBus;
// TODO: 在此处引用程序要求的附加头文件
DWORD WINAPI HostB(LPVOID);
DWORD WINAPI HostA(LPVOID);
