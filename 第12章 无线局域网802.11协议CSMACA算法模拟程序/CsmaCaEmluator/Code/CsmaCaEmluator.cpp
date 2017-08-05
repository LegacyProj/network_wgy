// CsmaCaEmluator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{

	dwBus=0;
	DWORD ThreadID=0;
	CreateThread(NULL,0,HostB,NULL,0,&ThreadID);
	CreateThread(NULL,0,HostA,NULL,0,&ThreadID);
	srand((unsigned)time(NULL)); 
	for(int i=0;i<10000;i++)
	{
		Sleep((rand()%20)*300);
		dwBus=dwBus|CONF;
		Sleep(2000);
		dwBus=0;
	}
	return 0;
}

